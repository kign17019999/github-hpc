void branch_and_bound(int *path, int path_cost, int *visited, int level, int rank, int num_procs) {
    count_bb+=1;
    if (level == n) {
        for(int i=0; i<MAX_CITIES; i++){
            if(all_best_cost>best_path_cost[i]) all_best_cost=best_path_cost[i];
        }
        if (path_cost < all_best_cost) {
            best_path_cost[rank] = path_cost;
            for (int i = 0; i < n; i++) best_path[rank][i] = path[i];

            for(int i = 0; i < num_procs; i++) {
                if(i != rank) {
                    MPI_Request request;
                    MPI_Isend(&path_cost, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &request);
                }
            }
        }
    } else {
        for (int i = 0; i < n; i++) {
            if (!visited[i]) {
                    path[level] = i;
                    visited[i] = 1;
                    int new_cost = path_cost + dist[i][path[level - 1]];
            
            if (new_cost < all_best_cost) {
                        branch_and_bound(path, new_cost, visited, level + 1, rank, num_procs);
                    }
                    visited[i] = 0;
            }
        }
    }
    MPI_Status status;
    int incoming_cost;
    MPI_Irecv(&incoming_cost, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &request);
    if(MPI_Test(&request, &flag, &status)) {
        if(incoming_cost < all_best_cost) {
            all_best_cost = incoming_cost;
        }
    }
}
