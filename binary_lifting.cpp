#include <bits/stdc++.h>
using namespace std;

#define FAST  ios_base::sync_with_stdio(false); cin.tie(0);
 

int main()
{
    FAST
    int n , m;
    cin >> n >> m;
    vector < vector < array < int , 2 >  > > adj(n+1);
    for ( int i = 0 ; i < n - 1 ; i++){
        int x , y , a;
        cin >> x >> y >> a;
        adj[x].push_back({y,a});
        adj[y].push_back({x,a});
    }
    int lg = 20;
    vector < vector < array < int , 2 >  > > up(n+1 , vector< array < int , 2 >  > (lg , {-1 , (int)1e9 + 10}));
    vector < int > depth(n+1,0);
    function < void (int , int) > dfs = [&] (int u  , int p){
        for ( auto [x , y ] : adj[u]){
            if ( x == p ) continue;
            up[x][0][0] = u;
            up[x][0][1] = y;
            depth[x] = depth[u] + 1;
            dfs(x , u);
        }
    };
    up[1][0][0] = -1;
    dfs(1 , -1);
    for ( int i = 1 ; i < lg ; i++){
        for ( int j = 1 ; j <= n ; j++){
            if ( up[j][i-1][0] != -1 ) {
                up[j][i][0] = up[up[j][i-1][0]][i-1][0];
                up[j][i][1] = min(up[j][i-1][1] , up[up[j][i-1][0]][i-1][1]);
            }
        }
    }
    int q;
    cin >> q;
   while (q--) {
    int a, b;
    cin >> a >> b;
    int a1 = a, b1 = b;

    if (depth[a] < depth[b]) swap(a, b);

    // Bring 'a' up to depth of 'b'
    int k = depth[a] - depth[b];
    for (int i = lg - 1; i >= 0; i--) {
        if ((1 << i) <= k) {
            a = up[a][i][0];
            k -= (1 << i);
        }
    }

    int lca;
    if (a == b) {
        lca = a;
    } else {
        for (int i = lg - 1; i >= 0; i--) {
            if (up[a][i][0] != -1 && up[a][i][0] != up[b][i][0]) {
                a = up[a][i][0];
                b = up[b][i][0];
            }
        }
        lca = up[a][0][0];
    }

    int ans = INT_MAX;

    // Path from 'a1' to 'lca'
    int dist_a = depth[a1] - depth[lca];
    for (int i = lg - 1; i >= 0; i--) {  
        if ((dist_a & (1 << i))) {
            ans = min(ans, up[a1][i][1]);
            a1 = up[a1][i][0];
        }
    }

    // Path from 'b1' to 'lca'
    int dist_b = depth[b1] - depth[lca];
    for (int i = lg - 1; i >= 0; i--) { 
        if ((dist_b & (1 << i))) {
            ans = min(ans, up[b1][i][1]);
            b1 = up[b1][i][0];
        }
    }

    cout << ans << '\n';
}


   
}  
