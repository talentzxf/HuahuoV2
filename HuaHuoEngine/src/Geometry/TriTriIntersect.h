#pragma once

int tri_tri_intersect_with_isectline(
    float V0[3], float V1[3], float V2[3],
    float U0[3], float U1[3], float U2[3], int *coplanar,
    float isectpt1[3], float isectpt2[3]);
