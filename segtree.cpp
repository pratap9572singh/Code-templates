#include <bits/stdc++.h>
using namespace std;
using ll = long long;
#define FAST  ios_base::sync_with_stdio(false); cin.tie(0);
const int mod = 1e9 + 7;
 
void setIO(string name = "")
{
    FAST;
    if (name.size())
    {
        (void)!freopen((name + ".in").c_str(), "r", stdin);
        (void)!freopen((name + ".out").c_str(), "w", stdout);
    }
}
 
struct item {
    ll sum;
};
 
 
struct segtree{
    
    item NEUTRAL_ELEMENT = {0};
 
    item merge( item x , item y){
        return {   
            max(x.sum , y.sum)
        };
    }
 
    item single (int v){
        item x ;
        x.sum = v;
        return x;
    }
 
    int size;
    vector < item > values;
    void init( int n ){     
        size = 1;
        while(size < n) size *= 2;
        values.assign(2*size, NEUTRAL_ELEMENT);
    }
 
 
    void build( vector < int > &a , int x , int lx , int rx){
        if( rx - lx == 1) {
            if ( lx < (int)a.size()){
                values[x] = single(a[lx]);
            }
            return;
        }
        int m = (lx + rx)/2;
        build(a , 2*x + 1 , lx , m);
        build (a , 2*x + 2 ,m, rx);
        values[x] = merge(values[2*x + 1] , values[2*x + 2]);
    }
    void build(vector < int > &a){
        build(a , 0 , 0 , size);
    }
    void set( int i , int v , int x , int lx , int rx){
        if (rx - lx == 1) {
            values[x] = single(v);
            return;
        }
        int m = (lx + rx)/2;
        if ( i < m){
            set(i , v , 2*x + 1 , lx , m);
        }
        else {
            set ( i , v , 2*x + 2 , m , rx);
        }
        values[x] = merge(values[2*x + 1] ,values[2*x + 2]);
    }
 
    void set( int i , int v){
        set( i , v , 0 , 0 , size);
    }
    int calc ( int k , int x , int lx , int rx){
        if ( values[x].sum < k ) return -1;
        if(rx - lx == 1) return lx;
        int m = (rx + lx)/2;
        if ( values[2*x + 1].sum >= k) return calc(k , 2*x + 1 , lx , m);
        return calc(k , 2*x + 2 , m , rx);
    }
 
    int calc( int k ){
        return calc( k , 0 , 0 , size);
    }
 
};
 
int main(){
    int n , m ;
    cin >> n >> m;
    vector < int > a(n);
    for ( int &i : a) cin >> i;
    segtree st;
    st.init(n);
    st.build(a);
    while(m--){
        int op;
        cin >> op;
        if (op == 1){
            int i , v;
            cin >> i >> v;
            st.set(i,v);
        }
        else{
            int x;
            cin >> x;
            cout << st.calc(x) << endl;
        }
    }
}
