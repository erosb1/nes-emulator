#include "console.h"

int main() {
    // använd macrot print istället för printf, eftersom RISC-V bräden inte har tillgång till printf
    print("Hello world %i", 23);

    return 0;
}