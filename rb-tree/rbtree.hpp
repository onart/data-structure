#include <cstdlib>
namespace onart{

    template<class T>
    inline void swap(T& a, T& b){
        T c = b;
        b = a;
        a = c;
    }

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
                if(cur) while(1){
                    const _key_t& ck = getKey(cur->value);
                    if(ck < k) { 
                        if(cur->right){ cur = cur->right; }
                        else{
                            __RBT_Node* newNode = allocate();
                            newNode->red = true;
                            cur->insertRight(newNode);
                            newNode->value.first = k;
                            return newNode->value.second;
                        }
                    }
                    else if(k < ck) { 
                        if(cur->left) cur = cur->left;
                        else{
                            __RBT_Node* newNode = allocate();
                            newNode->red = true;
                            cur->insertLeft(newNode);
                            newNode->value.first = k;
                            return newNode->value.second;
                        }
                    }
                    else return cur->value.second;
                }
                else{
                    root = allocate();
                    root->parent = nullptr;
                    root->left = nullptr;
                    root->right = nullptr;
                    root->red = false;
                    root->next = &endNode;
                    root->prev = &endNode;
                    root->value.first = k;
                    return root->value.second;
                }
            }

            inline iterator erase(iterator& i){
                if(i._container != this || i == end()){ return; }
                iterator ret = i;
                ++ret;
                i.node->remove();
                i.node->value.~T();
                deallocate(i.node);
                return ret;
            }

            inline iterator erase(const _key_t& k){
                iterator i = find(k);
                return erase(i);
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
                inline bool isLeft(){
                    //if(!parent) return false;
                    return parent->left == this;
                }
                inline __RBT_Node* grandParent() {
                    //if(!parent) return nullptr;
                    return parent->parent;
                }
                inline __RBT_Node* sibling() {
                    //if(!parent) return nullptr;
                    return reinterpret_cast<__RBT_Node*>(
                        (unsigned long long)(parent->left) ^ (unsigned long long)(parent->right) ^ (unsigned long long)(this)
                    );
                }
                inline __RBT_Node* uncle(){
                    //if(!parent) return nullptr;
                    return parent->sibling();
                }

                inline void rotateLeft(){
                    bool wasLeft = isLeft();
                    __RBT_Node* gp = parent;
                    parent = right;
                    right->parent = gp;
                    right = parent->left;
                    if(right) right->parent = this;
                    if(wasLeft){ gp->left = parent; }
                    else{ gp->right = parent; }
                    parent->left = this;
                }

                inline void rotateRight(){
                    bool wasLeft = isLeft();
                    __RBT_Node* gp = parent;
                    parent = left;
                    left->parent = gp;
                    left = parent->right;
                    if(left) left->parent = this;
                    if(wasLeft){ gp->left = parent; }
                    else{ gp->right = parent; }
                    parent->right = this;
                }

                inline void rbbalance(__RBT_Node* newNode){
                    if(red){
                        __RBT_Node* newUncle = sibling();
                        if(newUncle && newUncle->red) {
                            red = false;
                            newUncle->red = false;
                            if(parent->parent){
                                parent->red = true;
                                parent->parent->rbbalance(parent);
                            }
                            else{
                                return;
                            }
                        }
                        else{
                            bool newIsLeft = newNode->isLeft();
                            bool thisIsLeft = isLeft();
                            __RBT_Node* midNode = this;
                            if(!newIsLeft && thisIsLeft){ rotateLeft(); midNode = parent; newNode = this; }
                            else if(newIsLeft && !thisIsLeft){ rotateRight(); midNode = parent; newNode = this; }
                            midNode->red = false;
                            midNode->parent->red = true;
                            if(midNode->isLeft()){ midNode->parent->rotateRight(); }
                            else{ midNode->parent->rotateLeft(); }
                        }
                    }
                }

                inline void rbbalanceR() {

                }

                inline void insertRight(__RBT_Node* newRight){
                    right = newRight;
                    right->red = true;
                    right->parent = this;
                    right->left = nullptr;
                    right->right = nullptr;
                    right->prev = this;
                    if(next){ next->prev = right; }
                    right->next = next;
                    next = right;
                    rbbalance(right);
                }

                inline __RBT_Node* insertLeft(__RBT_Node* newLeft) {
                    left = newLeft;
                    left->red = true;
                    left->parent = this;
                    left->left = nullptr;
                    left->right = nullptr;
                    left->next = this;
                    left->prev = this;
                    if(prev){ prev->next = left; }
                    left->prev = prev;
                    prev = left;
                    rbbalance(left);
                }

                inline void remove(){
                    if(!parent){
                        return;
                    }
                    if(left && right){ // we want to preserve all iterators which didn't removed
                        bool thisIsLeft = isLeft();
                        bool prevIsLeft = prev->isLeft();
                        swap(parent, prev->parent);
                        if(thisIsLeft) prev->parent->left = prev; else prev->parent->right = prev;
                        if(prevIsLeft) parent->left = this; else parent->right = this;
                        swap(left, prev->left);
                        if(left) left->parent = this;
                        if(prev->left) prev->left->parent = prev;
                        swap(right, prev->right);
                        if(right) right->parent = this;
                        if(prev->right) prev->right->parent = prev;
                    }
                    if(prev){
                        if(next) next->prev = prev;
                        prev->next = next;
                    }
                    if(next){
                        //if(prev) prev->next = next;
                        next->prev = prev;
                    }
                    rbbalanceR();
                }
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
}