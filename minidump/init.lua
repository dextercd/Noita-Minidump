local ffi = require("ffi")

ffi.cdef([[

void* LoadLibraryA(const char*);

]])

assert(ffi.C.LoadLibraryA("mods/minidump/minidump.dll") ~= nil)
