/*
 * Grand Theft Auto 6 Xbox 360 — native demake bootstrap.
 * This is an early Xenon target smoke test, not gameplay code.
 */

volatile unsigned int gta6_bootstrap_marker = 0x47544136u;

void gta6_x360_bootstrap(void)
{
    /* Keep a visible side effect so the linker retains this function. */
    gta6_bootstrap_marker ^= 0x00000001u;
}
