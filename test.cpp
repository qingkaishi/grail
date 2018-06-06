/*
 * test.cpp
 *
 *  Created on: Jun 6, 2018
 *      Author: parallels
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "PushPopCache.h"

void add(CFLStack& S, int i) {
    printf("add %d...", i);
    bool ret = S.add(i);
    if (ret)
        printf("success\n");
    else
        printf("fail\n");
}

void test_1 () {
    CFLStack CFLSolver;
    add(CFLSolver, -100);
    add(CFLSolver, -200);
    add(CFLSolver, 300);
    add(CFLSolver, 1);
    add(CFLSolver, 2);
    add(CFLSolver, -2);
    add(CFLSolver, 3);
    add(CFLSolver, 4);
    add(CFLSolver, -4);
    add(CFLSolver, -3);
    add(CFLSolver, -1);
    add(CFLSolver, 7);
    add(CFLSolver, -7);
//    add(CFLSolver, -8);
//    add(CFLSolver, -9);
//    add(CFLSolver, -10);
    add(CFLSolver, 11);
    add(CFLSolver, -11);
    add(CFLSolver, 12);
    add(CFLSolver, 13);
    add(CFLSolver, 14);
    add(CFLSolver, 15);
    add(CFLSolver, -15);
    add(CFLSolver, 16);
    add(CFLSolver, -16);
    add(CFLSolver, 17);
    add(CFLSolver, 18);
    add(CFLSolver, -18);
    add(CFLSolver, 19);
    add(CFLSolver, 20);
    add(CFLSolver, 21);
    add(CFLSolver, -21);
    add(CFLSolver, 22);
    add(CFLSolver, -22);
    add(CFLSolver, -20);
    add(CFLSolver, -19);
    add(CFLSolver, -17);
    add(CFLSolver, -14);
    add(CFLSolver, -13);
    add(CFLSolver, -12);
    add(CFLSolver, -300);
}

void pop(CFLStack& S, int Num) {
    while (Num-- > 0) {
        printf("pop!\n");
        S.pop();
    }
}

void test_2() {
    CFLStack CFLSolver;
    CFLSolver.push(); add(CFLSolver, -100);
    CFLSolver.push(); add(CFLSolver, -200);
    CFLSolver.push(); add(CFLSolver, 1);
    CFLSolver.push(); add(CFLSolver, 2);
    CFLSolver.push(); add(CFLSolver, -2);
    CFLSolver.push(); add(CFLSolver, 3);
    CFLSolver.push(); add(CFLSolver, 4);
    CFLSolver.push(); add(CFLSolver, -4);
    CFLSolver.push(); add(CFLSolver, -3);
    CFLSolver.push(); add(CFLSolver, -1);
    add(CFLSolver, 7);
    add(CFLSolver, -7);

    pop(CFLSolver, 9);
    add(CFLSolver, -3);add(CFLSolver, -1);

    CFLSolver.push(); add(CFLSolver, -100);
    CFLSolver.push(); add(CFLSolver, -200);
    CFLSolver.push(); add(CFLSolver, 1);
    CFLSolver.push(); add(CFLSolver, 2);
    CFLSolver.push(); add(CFLSolver, -2);
    CFLSolver.push(); add(CFLSolver, 3);
    CFLSolver.push(); add(CFLSolver, 4);
    CFLSolver.push(); add(CFLSolver, -4);
    CFLSolver.push(); add(CFLSolver, -3);
    CFLSolver.push(); add(CFLSolver, -1);
    add(CFLSolver, 7);
    add(CFLSolver, -7);
}

int main() {
    test_2();
    return 0;
}
