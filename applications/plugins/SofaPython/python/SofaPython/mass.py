import Sofa
import numpy
import numpy.linalg

from SofaPython import Quaternion

class RigidMassInfo:
    def __init__(self):
        self.mass=0.
        self.com=[0.,0.,0.]
        self.diagonal_inertia=[0.,0.,0.]
        self.inertia_rotation=Quaternion.id()

    def setFromMesh(self, filepath, density = 1, scale3d=[1,1,1]):
        rigidInfo = Sofa.generateRigid( filepath, density, scale3d[0], scale3d[1], scale3d[2] )
        self.mass = rigidInfo[0]
        self.com = rigidInfo[1:4]
        self.diagonal_inertia = rigidInfo[4:7]
        self.inertia_rotation = rigidInfo[7:11]

    def getWorldInertia(self):
        """ @return inertia with respect to world reference frame
        """
        R = Quaternion.to_matrix(self.inertia_rotation)
        # I in world axis
        I = numpy.dot(R.transpose(), numpy.dot(numpy.diag(self.diagonal_inertia), R))
        # I at world origin, using // axis theorem
        # see http://www.colorado.edu/physics/phys3210/phys3210_sp14/lecnotes.2014-03-07.More_on_Inertia_Tensors.html
        # or https://en.wikipedia.org/wiki/Moment_of_inertia
        a=numpy.array(self.com).reshape(3,1)
        return I + self.mass*(pow(numpy.linalg.norm(self.com),2)*numpy.eye(3) - a*a.transpose())

    def __add__(self, other):
        res = RigidMassInfo()
        # sum mass
        res.mass = self.mass+other.mass
        # barycentric center of mass
        res.com = (self.mass*numpy.array(self.com) + other.mass*numpy.array(other.com)) / ( self.mass + other.mass )

        # inertia tensors
        # resultant inertia in world frame
        res_I_w = self.getWorldInertia() + other.getWorldInertia()

        # resultant inertia at com, world axis, using // axis theorem
        a=numpy.array(res.com).reshape(3,1)
        res_I_com = res_I_w - res.mass*(pow(numpy.linalg.norm(res.com),2)*numpy.eye(3) - a*a.transpose())

        U, res.diagonal_inertia, V = numpy.linalg.svd(res_I_com)
        # det should be 1->rotation or -1->reflexion
        if numpy.linalg.det(U) < 0 : # reflexion
            # made it a rotation by negating a column
            U[:,0] = -U[:,0]
        res.inertia_rotation = Quaternion.from_matrix( U )

        return res