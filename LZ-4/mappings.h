
// implementation of the different mappings of LZ-index

#include "bitmap.h"
#include "revtrie.h"
#include "lztrie.h"


inline uint NODE(lztrie T, uint id);

inline uint RNODE(revtrie RT, uint id);

inline uint IDS(lztrie T, uint pos);

inline uint RIDS(revtrie RT, uint rpos);

