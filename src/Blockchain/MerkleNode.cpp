#include <string>
#include <memory>

#include "MerkleNode.h"

namespace IASLib
{
    template <typename T, const char *(hash_func)(const T&), size_t hash_len>
    bool MerkleNode<T, hash_func, hash_len>::validate() const {
        // If either child is not valid, the entire subtree is invalid too.
        if (left && !left->validate()) {
            return false;
        }
        if (right && !right->validate()) {
            return false;
        }

        CString valHash;
        if (hasChildren()) {
            valHash = computeHash();
        } else {
            valHash = hash_func(*value);
        }
        return valHash == hash;
    }
}