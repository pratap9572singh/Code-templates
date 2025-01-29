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
    int m , c;
};
 
 
struct segtree{
    
    item NEUTRAL_ELEMENT = {INT_MAX ,0};
 
    item merge( item x , item y){
        if ( x.m < y.m) return x;
        if ( x.m > y.m) return y;
        return {x.m , x.c + y.c};
    }
 
    item single (int v){
        return {v,1};
    }
 
    int size;
    vector < item > values;
    void init( int n ){
        size = 1;
        while(size < n) size *= 2;
        values.resize(2*size);
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
 
    item calc (int l , int r , int x , int lx , int rx){
        if( lx >= r || l >= rx ) return NEUTRAL_ELEMENT;
        if(lx >= l && rx <= r ) return values[x];
        int m = (lx + rx)/2;
        item s1 = calc(l, r , 2*x + 1 , lx , m);
        item s2 = calc(l , r , 2*x + 2 , m , rx);
        return merge(s1 , s2);
 
    }
 
    item calc(int l , int r){
        return calc(l, r , 0 , 0 , size);
    }
};
 
