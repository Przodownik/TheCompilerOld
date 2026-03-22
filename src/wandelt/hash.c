#include "hash.h"

u64 fnv1a_hash(StringView str, u64 count)
{
	u32 hash = 2166136261u; // FNV offset basis

	for (u64 i = 0; i < str.len; i++)
	{
		hash ^= (u8)str.data[i];
		hash *= 16777619u; // FNV prime
	}
	
    return hash % count;
}
