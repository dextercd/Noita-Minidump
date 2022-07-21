local ffi = require("ffi")

ffi.cdef([[

void* LoadLibraryA(const char*);

void init_minidump();

]])

local dll_path = "mods/minidump/minidump.dll"

assert(ffi.C.LoadLibraryA(dll_path) ~= nil)
local lib = ffi.load(dll_path)
lib.init_minidump()
