# # CST Continuum Elements
# (c) Wolfgang Flachberger

import numpy
import sympy

XI = numpy.asarray(sympy.symbols('xi:' + str(2))) # symbolic vector of local coordinates

a, b  = sympy.symbols('a, b') # element side lengths

"""
           (0)___a____(1)
            |         /
 CST:       |       /
           b|     /                         CS:        __ x (i)
            |   /                                     |
            | /                                       z
           (2)                                       (ii)

There are two translational degrees of freedom at each node => 6 DOFs.
The first DOF is the x-displacement at node (0), the second DOF is
the z-displacement at node (0), etc.

"""

def interpolationFunctions(XI, a, b):

        h = sympy.ones(3,1) # initialize symbolic column vector for shape functions

        # shape functions: one per node
        h[0] =  1 - XI[0]/a - XI[1]/b
        h[1] =  XI[0]/a
        h[2] =  XI[1]/b

        return h

h = interpolationFunctions(XI, a, b)


from IPython.display import display
print("Local coordinates:")
print("------------------")
for i in range(len(XI)):
    display(XI[i])
print("Shape functions:")
print("----------------")
display(h)

def interpolationFunctionsDerivatives(XI, h):
        """ interpolationfunction[node, derivativeaxis] """
        dh_dX = sympy.ones(3, 2)
        for index in range(3):
            for axis in range(2):
                dh_dX[index,axis] = sympy.diff(h[index], XI[axis])
        return dh_dX

dh_dx = interpolationFunctionsDerivatives(XI, h)

print("Shape function derivatives:")
print("---------------------------")
display(dh_dx)

X = numpy.asarray(sympy.symbols('x:' + str(2)))

u, v, u_x, u_z, v_x, v_z = sympy.symbols('u, v, u_x, u_z, v_x, v_z')

# parameters
E     = sympy.Symbol('E')
nu    = sympy.Symbol('nu')

lmbda = (E*nu)/((1+nu)*(1-2*nu))
mu    = E/(2*(1+nu))

u     = numpy.array([u, v])                   # u (displacement field)
du_dx = numpy.array([[u_x, u_z], [v_x, v_z]]) # grad(u)


def epsilon(grad_u):
    """ returns strain tensor """
    return 0.5 * ( grad_u + grad_u.T )

def sigma(epsilon):
    """ computes stress tensor from strain tensor """
    return lmbda*numpy.trace(epsilon)*numpy.eye(2) + 2*mu*epsilon


# strain energy density
W = numpy.trace(sigma(epsilon(du_dx))@epsilon(du_dx))


print(" Strain energy density:")
print("-----------------------")
display(W)

U_ = numpy.asarray(sympy.symbols('U:' + str(6)))

W = W.subs([ (u_x, (U_[0] * dh_dx[0,0] + U_[2] * dh_dx[1,0] + U_[4] * dh_dx[2,0]) ),
             (v_x, (U_[1] * dh_dx[0,0] + U_[3] * dh_dx[1,0] + U_[5] * dh_dx[2,0]) ),
             (u_z, (U_[0] * dh_dx[0,1] + U_[2] * dh_dx[1,1] + U_[4] * dh_dx[2,1]) ),
             (v_z, (U_[1] * dh_dx[0,1] + U_[3] * dh_dx[1,1] + U_[5] * dh_dx[2,1]) ) ])


print(" Strain energy density as a function of DOFs:")
print("----------------------------------------------")
display(W)

# Integrate over x from 0 to (a-z), then over z from 0 to b
W = sympy.integrate(W, ( XI[0], 0, (a-XI[1]) ))
W = sympy.integrate(W, ( XI[1], 0,  b        ))
display(W)

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
