#!/usr/bin/python3

import math

POINTS = 20

WH_LVL = 210
BL_LVL = 40

WH_DIFF = 6
GR_DIFF = 2
N = 0
Knorm = 4
CEIL = 0.2

L_DIFF = 5/8.

def eval_coef():
    wh_coef = []
    gr_coef = []
    for x in range(1,POINTS+1):
        x = x/float(POINTS)
        wk = (1/x) + (WH_DIFF -1) + (1-x)*N
        #wk = WH_DIFF*((2-x)**(EXP-x))
        #wk = ((WH_DIFF/x - (WH_DIFF - 1))**EXP) + (WH_DIFF - 1)
        wk = math.ceil(wk - CEIL)
        #gk = GR_DIFF*((2-x)**(EXP-x))
        #gk = ((GR_DIFF/x - (GR_DIFF - 1))**EXP) + (GR_DIFF - 1)
        gk = (1/x) + (GR_DIFF -1) + (1-x)*N
        gk = math.ceil(gk - CEIL)
        wh_coef.append(wk)
        gr_coef.append(gk)
    #wh_coef.insert(0, wh_coef[0]+1)
    #gr_coef.insert(0, gr_coef[0]+1)
    return (wh_coef, gr_coef)

if __name__ == "__main__":
    wh_coef, gr_coef = eval_coef()
    wh_coef.reverse()
    gr_coef.reverse()
    print("wh_coef:", wh_coef)
    print("gr_coef:", gr_coef)
