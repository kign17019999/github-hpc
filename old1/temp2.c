
void branch_and_bound(int *path, int path_cost, int *visited, int level, int rank, int size) {
    count_bb+=1;
    if(count_bb<=0 && rank==1){
        printf("rank: %d| all=%d | local = %d \n",  rank, all_best_cost, best_path_cost[rank]);
    }
    if (level == n) {
        //for(int i=0; i<MAX_CITIES; i++){
        //     if(all_best_cost>best_path_cost[i]) all_best_cost=best_path_cost[i];
        //}
        //if(all_best_cost>best_path_cost[rank]) all_best_cost=best_path_cost[rank];
        if (path_cost < all_best_cost) {
            if(rank==0) printf("path cost %d\n", path_cost);
            best_path_cost[rank] = path_cost;
            all_best_cost = path_cost;
            for (int i = 0; i < n; i++) best_path[rank][i] = path[i];

            for(int i = 0; i < size; i++) {
                if(i != rank) {
                    MPI_Request request;
                    MPI_Isend(&all_best_cost, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &request);
                }
            }
        }
    } else {
        for (int i = 0; i < n; i++) {
            if (!visited[i]) {
                    path[level] = i;
                    visited[i] = 1;
                    int new_cost = path_cost + dist[i][path[level - 1]];
                    //if(rank==0) printf("new= %d, all = %d\n",new_cost ,all_best_cost);
                    if (new_cost < all_best_cost) {
                        branch_and_bound(path, new_cost, visited, level + 1, rank, size);
                    }
                    visited[i] = 0;
            }
        }
    }
    //MPI_Status status;
    //MPI_Request request;
    if(1){
        for(int i=0; i<size;i++){
            if(i!=rank){
                int incoming_cost;
                int flag=0;
                MPI_Request request9;
                MPI_Status status;
                MPI_Irecv(&incoming_cost, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &request9);
                MPI_Test(&request9, &flag, MPI_STATUS_IGNORE);
                if(flag){
                    if(incoming_cost < all_best_cost && incoming_cost !=0) {
                        all_best_cost = incoming_cost;
                    }
 //                   if(1) printf("from %d with %d compare to %d  | local = %d\n", i, incoming_cost, all_best_cost, best_path_cost[rank]);
                }
            }
         }
    }
}
