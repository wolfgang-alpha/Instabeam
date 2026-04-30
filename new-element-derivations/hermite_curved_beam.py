# Instabeam - element stiffness matrix derivation
# Copyright (C) Wolfgang Flachberger
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# # Hermite Curved Beam Elements
# (c) Wolfgang Flachberger

import numpy
import sympy

phi, alpha, beta, R, E, I, A = sympy.symbols("varphi, alpha, beta, R, E, I, A ")

"""
HERMITE CURVED BEAM ELEMENTS:


             ____R____(1)
            |          |
 ARC:       |          /
           R|         /                         CS:     __ x (i)
            |       /                                  |
            | _ _ /                                    z
           (0)                                       (ii)

Depicted case: alpha = pi/2

DoF_0: radial translation.......Node (0)
DoF_1: rotation.................Node (0)
DoF_2: radial translation.......Node (1)
DoF_3: rotation.................Node (1)
DoF_4: tangential translation...Node (0)
DoF_5: tangential translation...Node (1)

"""

def interpolationFunctions(x,l):

        h = sympy.ones(6,1)

        h[0] =  1 - 3 * (x/l)**2 + 2 * (x/l)**3
        h[1] =  - x * (1 - (x/l))**2
        h[2] =  3 * (x/l)**2 - 2 * (x/l)**3
        h[3] =  - x * ((x/l)**2 -(x/l))
        h[4] = 1 - x/l
        h[5] = x/l

        return h

h = interpolationFunctions(phi,alpha)


from IPython.display import display
print("Shape functions:")
print("----------------")
display(h)


def interpolationFunctionsDerivatives(h, phi):
        """ interpolationfunction[node, derivativeaxis] """
        dh_dX = sympy.ones(6, 2)
        for fun in range(6):
            dh_dX[fun,0] = sympy.diff(h[fun], phi)
            dh_dX[fun,1] = sympy.diff(dh_dX[fun,0], phi)
        return dh_dX

dh_dx = interpolationFunctionsDerivatives(h, phi)

print("Shape function derivatives:")
print("---------------------------")
display(dh_dx)

U_ = numpy.asarray(sympy.symbols('U:' + str(6)))

w    = U_[0] *     h[0]   + U_[1] *     h[1]   + U_[2] *     h[2]   + U_[3] *     h[3]    # w
w_xx = U_[0] * dh_dx[0,1] + U_[1] * dh_dx[1,1] + U_[2] * dh_dx[2,1] + U_[3] * dh_dx[3,1]  # w''
u_x  = U_[4] * dh_dx[4,0] + U_[5] * dh_dx[5,0]                                            # u'

""" See PARKUS p.247 """
M = (E*I*( u_x - w_xx ))/(R**2)
N = (E*A*( w_xx + w + (M*R**2)/(E*I) ))/R


W = M**2/(E*I) + N**2/(E*A) # complementary energy density

display(W)

W = sympy.integrate( W*R, ( phi, 0, alpha ) ) # complementary energy

esm = sympy.zeros(6,6)
for i in range(6):
    for j in range(6):
        esm[i,j] = sympy.diff( sympy.diff(W, U_[i]), U_[j] )

print(" ESM: ")
print("------")
display(esm)

ESM = numpy.asarray(esm)
for index in numpy.ndindex(6,6):
    print("k_e_local",index,"=",ESM[index])
