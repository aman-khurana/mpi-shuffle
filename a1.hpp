/*  AMAN
 *  KHURANA
 *  AMANKHUR
 */

#ifndef A1_HPP
#define A1_HPP

#include <mpi.h>
#include <iostream>
#include <vector>
#include <random>
#include <unordered_map>
#include <functional>
#include <numeric> 
#include <algorithm>

template <typename T, typename Hash>
void mpi_shuffle(std::vector<T>& inout, Hash hash, MPI_Datatype Type, MPI_Comm Comm) {

int rank,size;
MPI_Comm_rank(Comm, &rank);
MPI_Comm_size(Comm, &size);

// putting the input items in send buffer
std::vector<T> send = inout;

// hash values of the items in the send buffer
std::vector<int> hash_send(send.size());

for(int i=0; i<hash_send.size(); i++)
        hash_send[i] = hash(send[i])%size;

// new structure with send value and it's corresponding hash
// used to sort the values based on hash
struct value_hash{
        int value;
        int hash;
    };

std::vector<value_hash> value_hash_vector;

// populating the value-hash vector
for(int i = 0; i<send.size(); i++){

    value_hash vh;
    vh.value = send[i];
    vh.hash  = hash_send[i];
    value_hash_vector.push_back(vh);
}

// sorting the value-hash vector based on hash value 
std::sort(value_hash_vector.begin(), value_hash_vector.end(), 
                [](auto const &a, auto const &b){return a.hash < b.hash; });

std::vector<int> send_by_hash(send.size());
// the send buffer arranged by hash value, used as send buffer in Alltoallv() call
for(int i=0; i< send_by_hash.size(); i++)
        send_by_hash[i] = value_hash_vector[i].value;

// map for storing (hash value, count) pairs
// counting number of send objects per processor
std::unordered_map<int,int> send_counts_map;
for(int const &i: hash_send)
    send_counts_map[i]++;

// send counts per processor, used in Alltoallv()
std::vector<int> send_counts(size);

for(int i=0; i<size; i++)
    send_counts[i] = send_counts_map[i];
    


// preparing send displacements using non inclusive partial sum
// over send counts

std::vector<int> send_counts_scan(size);

std::partial_sum(send_counts.begin(), send_counts.end() ,
                send_counts_scan.begin());

// send offsets for each processor, used in Alltoallv()
std::vector<int> send_counts_prescan(size);

send_counts_prescan[0] = 0;
std::copy(send_counts_scan.begin(),send_counts_scan.end()-1, 
          send_counts_prescan.begin()+1);

// number of objects to be received from each processor
std::vector<int> recv_counts(size);

// exchanging send counts for each processor to get the recv counts 
MPI_Alltoall(send_counts.data(),1, MPI_INT,
            recv_counts.data(), 1, MPI_INT, MPI_COMM_WORLD);

// calculating offsets in recv buffer for receving from each processor
// using prescan

std::vector<int> recv_counts_scan(size);
std::partial_sum(recv_counts.begin(), recv_counts.end() ,
                 recv_counts_scan.begin());

// receive displacements in the receive buffer corresponding to each processor
std::vector<int> recv_counts_prescan(size);

recv_counts_prescan[0] = 0;
std::copy(recv_counts_scan.begin(),recv_counts_scan.end()-1, 
          recv_counts_prescan.begin()+1);

// calculating receive buffer size by summing all the recv counts
int recv_size = 0;  
recv_size = std::accumulate(recv_counts.begin(), recv_counts.end(), recv_size);

// receive buffer, used to receive data from all the processors
std::vector<T> recv(recv_size);

// exchanging the data
MPI_Alltoallv(send_by_hash.data(), &send_counts[0], &send_counts_prescan[0], MPI_INT,
             recv.data(),&recv_counts[0], &recv_counts_prescan[0], MPI_INT, MPI_COMM_WORLD);

inout = recv; 

} // mpi_shuffle

#endif // A1_HPP
