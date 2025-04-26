// solve.cpp
// Fully inlined, fast I/O, static adjacency arrays, bitset‑based small‑n solver,
// sort‑based medium‑n solver, linear‑scan large‑n solver.
// gcc/clang: -O3 -march=native -funroll-loops

#pragma GCC optimize("Ofast","unroll-loops","inline")
// #pragma GCC target("popcnt","bmi2")
#pragma GCC optimize("Ofast","unroll-loops","inline")


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
    for (; c>='0'&&c<='9'; c = (buf_pos<buf_len?buf[buf_pos++]:EOF))
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
    if(!fi||!fo) return 1;
    stdin = fi; stdout = fo;

    int n = readInt();
    int m = readInt();

    // allocate arrays
    head    = (int*)malloc((n)*sizeof(int));
    deg_cnt = (int*)malloc((n)*sizeof(int));
    nxt     = (int*)malloc((2*m)*sizeof(int));
    to_arr  = (int*)malloc((2*m)*sizeof(int));
    fill(head, head+n, -1);
    fill(deg_cnt, deg_cnt+n, 0);

    // read edges
    edge_cnt = 0;
    for(int i=0;i<m;i++){
        int u = readInt(), v = readInt();
        addEdge(u,v); deg_cnt[u]++;
        addEdge(v,u); deg_cnt[v]++;
    }

    vector<int> solution;
    solution.reserve(n/10);

    // regime thresholds
    const int SMALL_N  = 1000;
    const int MEDIUM_N = 100000;

    if (n <= SMALL_N) {
        // --- small: bitset greedy+prune ---
        int B = (n + 63)>>6;
        static uint64_t reach[SMALL_N][16];
        static uint64_t covered[16];
        static bool chosen[SMALL_N], keep[SMALL_N];
        memset(reach,0,sizeof reach);
        memset(covered,0,sizeof(uint64_t)*B);
        memset(chosen,0,n);
        // build reach bitsets
        for(int i=0;i<n;i++){
            reach[i][i>>6] |= 1ULL<<(i&63);
            for(int e=head[i];e!=-1;e=nxt[e]){
                int j = to_arr[e];
                reach[i][j>>6] |= 1ULL<<(j&63);
            }
        }
        int covered_cnt = 0;
        // greedy
        while(covered_cnt < n){
            int best=-1, best_gain=-1;
            for(int i=0;i<n;i++){
                if(chosen[i]) continue;
                int gain=0;
                // compute popcount of (reach[i] & ~covered)
                for(int k=0;k<B;k++){
                    uint64_t diff = reach[i][k] & ~covered[k];
                    gain += __builtin_popcountll(diff);
                }
                if(gain>best_gain){
                    best_gain=gain; best=i;
                }
            }
            chosen[best]=true;
            solution.push_back(best);
            // update covered & count
            for(int k=0;k<B;k++){
                uint64_t diff = reach[best][k] & ~covered[k];
                covered[k] |= reach[best][k];
                covered_cnt += __builtin_popcountll(diff);
            }
        }
        // prune
        int sz = solution.size();
        memset(keep,0,n);
        for(int p:solution) keep[p]=true;
        for(int idx=0;idx<sz;idx++){
            int p=solution[idx];
            keep[p]=false;
            // recompute cover count
            int cnt=0;
            for(int k=0;k<B;k++){
                uint64_t cov=0;
                for(int i=0;i<n;i++){
                    if(keep[i]) cov |= reach[i][k];
                }
                cnt += __builtin_popcountll(cov);
            }
            if(cnt<n) keep[p]=true;
        }
        // build final
        vector<int> final;
        final.reserve(sz);
        for(int i=0;i<n;i++) if(keep[i]) final.push_back(i);
        solution.swap(final);

    } else if (n <= MEDIUM_N) {
        // --- medium: degree‑greedy ---
        vector<int> order(n);
        for(int i=0;i<n;i++) order[i]=i;
        sort(order.begin(), order.end(),
             [&](int a,int b){ return deg_cnt[a]>deg_cnt[b]; });
        vector<char> powered(n,0);
        int rem = n;
        for(int u:order){
            if(rem==0) break;
            if(!powered[u]){
                solution.push_back(u);
                powered[u]=1; rem--;
                for(int e=head[u];e!=-1;e=nxt[e]){
                    int v=to_arr[e];
                    if(!powered[v]){
                        powered[v]=1; rem--;
                    }
                }
            }
        }
    } else {
        // --- large: linear cover ---
        vector<char> powered(n,0);
        for(int i=0;i<n;i++){
            if(!powered[i]){
                solution.push_back(i);
                powered[i]=1;
                for(int e=head[i];e!=-1;e=nxt[e]){
                    powered[to_arr[e]]=1;
                }
            }
        }
    }

    // write binary string
    string out(n,'0');
    for(int p:solution) out[p]='1';
    out.push_back('\n');
    fwrite(out.data(),1,out.size(), stdout);
    return 0;
}
