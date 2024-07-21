#include <string.h> //memcmp

bool sm_state_key_match(sm_state *s, const void *key, size_t key_length){
    if(key_length != s->key_length)
        return false;
    else
        return (memcmp(s->key, key, key_length) == 0);
}

int sm_state_set_key(sm_state *s, const void *key, size_t key_length) {
    memset(s->key, '\0', SM_STATE_HASH_KEYLEN);
    s->key_length = MIN(key_length, SM_STATE_HASH_KEYLEN);
    memcpy(s->key, key, MIN(key_length, s->key_length));
    return EXIT_SUCCESS;
}
    