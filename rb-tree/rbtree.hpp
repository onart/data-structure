#include <cstdlib>
namespace onart{

    template<class K, class V>
    struct kvpair{
        K first;
        V second;
    };

    template<class T>
    class set{
        private:
            template<class C>
            constexpr static inline bool IS_KVPAIR = false;
            template<class K, class V>
            constexpr static inline bool IS_KVPAIR<kvpair<K,V>> = true;
            constexpr static inline bool IS_MAP = IS_KVPAIR<T>;
            template<bool b> struct _ktsel;
            template<> struct _ktsel<true>{ using TYPE = decltype(T::first); using VALUETYPE = decltype(T::second); };
            template<> struct _ktsel<false>{ using TYPE = T; using VALUETYPE = void; };
            template<bool b, class T = void>
            struct __enableif{};
            template<class T>
            struct __enableif<true, T>{using TYPE=T;};
        public:
            using _key_t = _ktsel<IS_MAP>::TYPE;
            using _value_t = _ktsel<IS_MAP>::VALUETYPE;
            inline set(size_t reserve = 0):capacity(reserve),poolUsage(0){
                if(reserve){
                    reserved = reinterpret_cast<__RBT_Node*>(std::malloc((sizeof(__RBT_Node) + sizeof(__RBT_Node*)) * reserve));
                    istack = reinterpret_cast<__RBT_Node**>(reserved + reserve);
                    for(size_t i = 0; i < reserve; i++){
                        istack[i] = reserved + i;
                    }
                }
                count = 0;
            }
            inline ~set(){
                std::free(reserved);
            }

            inline unsigned long long size() const { return count; }
            inline iterator begin(){ return iterator(this, head); }
            inline const iterator cbegin() const { return iterator(this, head); }
            inline iterator end() const { return iterator(this, &endNode); }
            inline const iterator cend() const { return iterator(this, &endNode); }

            inline iterator find(const _key_t& k){
                __RBT_Node* cur = root;
                while(cur){
                    const _key_t& ck = getKey(cur->value);
                    if(ck < k) cur = cur->right;
                    else if(k < ck) cur = cur->left;
                    else return iterator(this, cur); 
                }
                return end();
            }

            inline const iterator find(const _key_t& k) const {
                __RBT_Node* cur = root;
                while(cur){
                    const _key_t& ck = getKey(cur->value);
                    if(ck < k) cur = cur->right;
                    else if(k < ck) cur = cur->left;
                    else return iterator(this, cur);
                }
                return end();
            }

            template<__enableif<IS_MAP, bool>::TYPE = true>
            inline _value_t& operator[](const key_t& k) const {
                __RBT_Node* cur = root;
                while(cur){
                    const _key_t& ck = getKey(cur->value);
                    if(ck < k) cur = cur->right;
                    else if(k < ck) cur = cur->left;
                    else return cur->value.second;
                }
            }

            class iterator{
                friend class set;
                public:
                    inline iterator():_container(nullptr),node(nullptr){}
                    inline iterator(const iterator& other):_container(other._container),node(other.node){}
                    inline bool operator==(const iterator& other) const { return node == other._node; }
                    inline bool operator!=(const iterator& other) const { return !operator==(other); }
                    inline iterator& operator++() { node = node->next; return *this; }
                    inline iterator operator++(int) { __RBT_Node* prv = node; node = node->next; return iterator(_container, prv); }
                    inline iterator& operator--() { node = node->prev; return *this; }
                    inline iterator operator--(int) { __RBT_Node* prv = node; node = node->prev; return iterator(_container, prv); }
                    inline T& operator*() { return node->value; }
                    inline const T& operator*() const { return node->value; }
                    inline T* operator->() { return &(node->value); }
                    inline const T* operator->() const { return &(node->value); }
                private:
                    inline iterator(set* c, __RBT_Node* n):_container(c),node(n){}
                    set* _container;
                    __RBT_Node* node;
            };
        private:
            struct __RBT_Node{
                T value;
                bool red;
                __RBT_Node* parent = nullptr;
                __RBT_Node* left = nullptr;
                __RBT_Node* right = nullptr;
                __RBT_Node* prev = nullptr;
                __RBT_Node* next = nullptr;
            };
            __RBT_Node* root = nullptr;
            __RBT_Node* head = nullptr;
            __RBT_Node endNode;
            __RBT_Node* reserved = nullptr;
            __RBT_Node** istack = nullptr;
            const unsigned long long capacity;
            unsigned long long poolUsage;
            unsigned long long count;
            inline static const _key_t& getKey(const T& n){
                if constexpr(IS_MAP){ return n.first; }
                else{ return n; }
            }
            inline __RBT_Node* allocate(){
                count++;
                if(capacity > poolUsage){ return istack[poolUsage++]; }
                else{ return reinterpret_cast<__RBT_Node*>(std::malloc(sizeof(__RBT_Node))); }
            }
            inline void deallocate(__RBT_Node* p){
                count--;
                if(p - reserved < 0 || p - reserved > capacity){ std::free(p); }
                else{ istack[poolUsage--] = p; }
            }
    };

    template<class K, class V>
    using map = set<kvpair<K,V>>;
    using isk = map<int, int>::_key_t;
    using ksk = set<float>::_value_t;
}