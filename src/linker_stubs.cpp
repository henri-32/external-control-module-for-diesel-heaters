//Einziges fehlendes Linker Symbol 
//Die Definition hier spart viel Toolchain Arbeit
extern "C" void __cxa_pure_virtual() {
    while (1);
}
extern "C" void yield(void) {}


