// solve.cpp
// Fully inlined, fast I/O, static adjacency arrays, bitset-based small-n solver,
// sort-based medium-n solver, linear-scan large-n solver.
// gcc/clang: -O3 -march=native -funroll-loops

#pragma GCC optimize("Ofast","unroll-loops","inline")
// #pragma GCC target("popcnt","bmi2")


#include <bits/stdc++.h>
using namespace std;

// fast buffered reader
static const size_t BUF_SZ = 1<<20;
static char buf[BUF_SZ];
static size_t buf_pos = 0, buf_len = 0;

static inline void refill() {
    buf_len = fread(buf,1,BUF_SZ, stdin);
    buf_pos = 0;
    if (!buf_len) buf[0] = EOF;
}

static inline int readInt() {
    int x = 0;
    char c;
    if (buf_pos >= buf_len) refill();
    c = buf[buf_pos++];
    while (c<'0'||c>'9') {
        if (buf_pos >= buf_len) refill();
        c = buf[buf_pos++];
    }
    for (; c>='0'&&c<='9'; c = (buf_pos<buf_len?buf[buf_pos++]:(char)EOF)) // Added (char)EOF cast for safety
        x = x*10 + (c - '0');
    return x;
}

// adjacency via head/nxt/to arrays
static int *head, *to_arr, *nxt;
static int *deg_cnt;
static int edge_cnt = 0;

// add undirected edge
inline void addEdge(int u, int v) {
    nxt[edge_cnt]  = head[u];
    to_arr[edge_cnt] = v;
    head[u] = edge_cnt++;
}

int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc!=3) {
        fprintf(stderr,"Usage: %s <in> <out>\n",argv[0]);
        return 1;
    }

    // redirect
    FILE* fi = fopen(argv[1],"rb");
    FILE* fo = fopen(argv[2],"wb");
    if(!fi||!fo) {
        fprintf(stderr, "Error opening files.\n"); // Added error message
        return 1;
    }
    // Assigning stdin/stdout directly is non-standard and potentially problematic.
    // Using freopen is a more standard way, but fread/fwrite below work directly with FILE*.
    // stdin = fi; // Commented out - use fi directly
    // stdout = fo; // Commented out - use fo directly

    // Temporarily assign stdin for readInt() - needs careful handling or refactoring readInt
    FILE* original_stdin = stdin;
    stdin = fi;
    int n = readInt();
    int m = readInt();
    stdin = original_stdin; // Restore original stdin


    // allocate arrays
    head    = (int*)malloc((n)*sizeof(int));
    deg_cnt = (int*)malloc((n)*sizeof(int));
    nxt     = (int*)malloc((2*m)*sizeof(int));
    to_arr  = (int*)malloc((2*m)*sizeof(int));

    // Check malloc results
    if (!head || !deg_cnt || !nxt || !to_arr) {
         fprintf(stderr, "Memory allocation failed.\n");
         fclose(fi); // Close files before exiting
         fclose(fo);
         // Free any successfully allocated memory before exiting
         free(head); free(deg_cnt); free(nxt); free(to_arr);
         return 1;
    }


    fill(head, head+n, -1);
    fill(deg_cnt, deg_cnt+n, 0);

    // read edges using readInt again
    edge_cnt = 0;
    stdin = fi; // Temporarily assign stdin again
    for(int i=0;i<m;i++){
        int u = readInt();
        int v = readInt();
        // Basic bounds check
        if (u < 0 || u >= n || v < 0 || v >= n) {
            fprintf(stderr, "Invalid node index read: u=%d, v=%d for n=%d\n", u, v, n);
             // Cleanup and exit
            fclose(fi); fclose(fo);
            free(head); free(deg_cnt); free(nxt); free(to_arr);
            stdin = original_stdin;
            return 1;
        }
        addEdge(u,v); deg_cnt[u]++;
        addEdge(v,u); deg_cnt[v]++;
    }
    stdin = original_stdin; // Restore original stdin
    fclose(fi); // Close input file now that reading is done


    vector<int> solution;
    solution.reserve(n/10 + 1); // Avoid division by zero if n=0, reserve slightly more

    // regime thresholds
    const int SMALL_N  = 1000;
    const int MEDIUM_N = 100000;

    if (n <= SMALL_N && n > 0) { // Added n > 0 check
        // --- small: bitset greedy+prune ---
        int B = (n + 63)>>6; // Calculate number of 64-bit blocks needed
        // Use vectors for dynamic allocation based on B
        vector<vector<uint64_t>> reach(n, vector<uint64_t>(B, 0));
        vector<uint64_t> covered(B, 0);
        vector<bool> chosen(n, false);
        vector<bool> keep(n, false); // Changed from static bool

        // build reach bitsets
        for(int i=0;i<n;i++){
            reach[i][i>>6] |= 1ULL<<(i&63); // Set bit for self
            for(int e=head[i];e!=-1;e=nxt[e]){
                int j = to_arr[e];
                 if (j >= 0 && j < n) { // Bounds check
                    reach[i][j>>6] |= 1ULL<<(j&63); // Set bit for neighbor
                 }
            }
        }

        int covered_cnt = 0;
        // greedy
        while(covered_cnt < n){
            int best=-1;
            long long best_gain=-1; // Use long long for gain

            for(int i=0;i<n;i++){
                if(chosen[i]) continue;
                long long gain=0;
                // compute popcount of (reach[i] & ~covered)
                for(int k=0;k<B;k++){
                    uint64_t diff = reach[i][k] & ~covered[k];
                    // Use __builtin_popcountll only if available, otherwise implement manually
                    #ifdef __GNUC__
                    gain += __builtin_popcountll(diff);
                    #else
                    // Manual popcount (or use std::popcount in C++20)
                    uint64_t count = 0;
                    while (diff > 0) {
                        diff &= (diff - 1);
                        count++;
                    }
                    gain += count;
                    #endif
                }
                if(gain>best_gain){
                    best_gain=gain;
                    best=i;
                }
            }

            // Check if a best node was found and gain is positive
            if (best == -1 || best_gain <= 0) {
                // This might happen if remaining nodes don't cover any new nodes.
                // To ensure termination, find *any* uncovered node and add it or its neighbor.
                int fallback_node = -1;
                for(int i=0; i<n; ++i) {
                   bool is_covered = (covered[i>>6] >> (i&63)) & 1ULL;
                   if (!is_covered) {
                      fallback_node = i;
                      break;
                   }
                }
                if (fallback_node != -1) {
                    best = fallback_node; // Choose the uncovered node itself
                    // Recalculate gain for this fallback node (though not strictly needed here)
                } else {
                   // Should not happen if covered_cnt < n, but break just in case
                   fprintf(stderr, "Warning: Greedy loop stuck despite uncovered nodes.\n");
                   break;
                }
            }


            chosen[best]=true;
            solution.push_back(best);

            // update covered & count
            for(int k=0;k<B;k++){
                uint64_t diff = reach[best][k] & ~covered[k];
                covered[k] |= reach[best][k];
                 #ifdef __GNUC__
                 covered_cnt += __builtin_popcountll(diff);
                 #else
                 // Manual popcount
                 uint64_t count = 0;
                 while (diff > 0) {
                    diff &= (diff - 1);
                    count++;
                 }
                 covered_cnt += count;
                 #endif
            }
        } // end while greedy

        // prune
        int sz = solution.size();
        // memset(keep,0,n); // keep is already initialized to false if using vector<bool>
        for(int p:solution) if (p>=0 && p<n) keep[p]=true; // Bounds check index

        for(int idx=0;idx<sz;idx++){
            int p=solution[idx];
             if (p < 0 || p >= n) continue; // Bounds check

            keep[p]=false;

            // recompute cover count
            int cnt=0;
            vector<uint64_t> temp_covered(B, 0); // Use temporary cover for check

            for(int i=0;i<n;i++){
                if(keep[i]) {
                   for(int k=0; k<B; ++k) {
                       temp_covered[k] |= reach[i][k];
                   }
                }
            }
             for(int k=0; k<B; ++k) {
                 #ifdef __GNUC__
                 cnt += __builtin_popcountll(temp_covered[k]);
                 #else
                 // Manual popcount
                 uint64_t diff = temp_covered[k];
                 uint64_t count = 0;
                 while (diff > 0) {
                    diff &= (diff - 1);
                    count++;
                 }
                 cnt += count;
                 #endif
             }

            if(cnt<n) keep[p]=true; // If removing p makes coverage incomplete, keep it
        } // end for prune


        // build final solution from keep flags
        vector<int> final_solution;
        final_solution.reserve(solution.size()); // Reserve estimated size
        for(int i=0;i<n;i++) {
            if(keep[i]) final_solution.push_back(i);
        }
        solution.swap(final_solution); // Replace original solution

    } else if (n <= MEDIUM_N && n > 0) { // Added n > 0 check
        // --- medium: degree-greedy ---
        vector<int> order(n);
        for(int i=0;i<n;i++) order[i]=i;
        sort(order.begin(), order.end(),
             [&](int a,int b){ return deg_cnt[a]>deg_cnt[b]; });

        vector<char> powered(n,0); // Use char for smaller memory footprint
        int rem = n;
        for(int u:order){
            if(rem==0) break;
            if(!powered[u]){
                solution.push_back(u);
                powered[u]=1; rem--;
                for(int e=head[u];e!=-1;e=nxt[e]){
                    int v=to_arr[e];
                    if(v >= 0 && v < n && !powered[v]){ // Bounds check
                        powered[v]=1;
                        rem--;
                    }
                }
            }
        }
    } else if (n > 0) { // Added n > 0 check
        // --- large: linear cover ---
        vector<char> powered(n,0);
        for(int i=0;i<n;i++){
            if(!powered[i]){
                solution.push_back(i);
                powered[i]=1;
                for(int e=head[i];e!=-1;e=nxt[e]){
                     int neighbor = to_arr[e];
                     if (neighbor >= 0 && neighbor < n) { // Bounds check
                        powered[neighbor]=1;
                     }
                }
            }
        }
    } // End n size checks

    // write binary string
    string out(n,'0');
    for(int p:solution) {
        if (p >= 0 && p < n) { // Bounds check before writing
            out[p]='1';
        }
    }
    out.push_back('\n');
    fwrite(out.data(),1,out.size(), fo); // Use fo directly

    fclose(fo); // Close output file

    // Free allocated memory
    free(head);
    free(deg_cnt);
    free(nxt);
    free(to_arr);

    return 0;
}
