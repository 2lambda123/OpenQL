/**
 * @file   unitary.h
 * @date   12/2018
 * @author Imran Ashraf
 * @author Anneriet Krol
 * @brief  unitary matrix (decomposition) implementation
 */

#ifndef _UNITARY_H
#define _UNITARY_H

#include <complex>
#include <string>

#include <ql/utils.h>
#include <ql/str.h>
#include <ql/gate.h>
#include <ql/exception.h>
#include <Eigen>
#include <unsupported/Eigen/MatrixFunctions>
#include <src/misc/lapacke.h>
namespace ql
{

class unitary
{
public:
    std::string name;
    std::vector<std::complex<double>> array;
    std::vector<std::complex<double>> SU;
    double delta;
    double alpha;
    double beta;
    double gamma;
    bool is_decomposed;
    std::vector<double> instructionlist;

    typedef Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> complex_matrix ;
    Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> matrix;

    unitary() : name(""), is_decomposed(false) {}
    unitary(std::string name, std::vector<std::complex<double>> array) : 
            name(name), array(array), is_decomposed(false)
    {
        DOUT("constructing unitary: " << name 
                  << ", containing: " << array.size() << " elements");
        // utils::print_vector(array,"[openql] unitary elements :"," , ");
        // TODO: add sanity checks for supplied arguments
    }

    double size()
    {
        return (double) array.size();
    }

    void decompose()
    {
        DOUT("decomposing Unitary: " << name);

    
        int matrix_size = (int)std::pow(array.size(),0.5);

        Eigen::Map<complex_matrix> matrix(array.data(), matrix_size, matrix_size);
         //      DOUT("constructing unitary: " << name << ", containing: " << matrix << " elements");
        
        // compute the number of qubits: length of array is collumns*rows, so log2(sqrt(array.size))
        int numberofbits = (int) log2(matrix_size);

        Eigen::MatrixXcd identity = Eigen::MatrixXcd::Identity(matrix_size, matrix_size);
        Eigen::MatrixXcd matmatadjoint = (matrix.adjoint()*matrix);
        bool equal = matmatadjoint.isApprox(identity, 0.00001);
        if( !equal)
        {
            //Throw an error
            EOUT("Unitary " << name <<" is not a unitary matrix!");

            throw ql::exception("Unitary '"+ name+"' is not a unitary matrix. Cannot be decomposed!"+to_string(matrix), false);
        }

        decomp_function(matrix.transpose(), numberofbits); //needed because the matrix is read in columnmajor
        // utils::print_vector(instructionlist, "Instruction list: ", "; ");

        
        DOUT("Done decomposing");
        is_decomposed = true;
    }

    std::string to_string(complex_matrix m, std::string vector_prefix = "",
                            std::string elem_sep = ", ")
    {
        std::ostringstream ss;
        ss << m << "\n";
        // ss << vector_prefix << " [";
        // Eigen::VectorXcd v = Eigen::Map<Eigen::VectorXcd>(m.data, m.size());
        // size_t sz = v.size();
        // if(sz > 0)
        // {
        //     size_t i;
        //     for (i=0; i<sz*sz-1; ++i)
        //         ss << v[i] << elem_sep;
        //     ss << v[i];
        // }

        // ss << "]";
        return ss.str();
    }


    void decomp_function(complex_matrix matrix, int numberofbits)
    {           
        if(numberofbits == 1)
        {
            std::vector<double> tmp(4);
            tmp = zyz_decomp(array);
            delta = tmp[0];
            alpha = tmp[1];
            beta  = tmp[2];
            gamma = tmp[3];
        }
        else
        {
            complex_matrix cc;
            complex_matrix ss;
            complex_matrix L0;
            complex_matrix L1;
            complex_matrix R0;
            complex_matrix R1;

            int n = matrix.rows()/2;
            // if q2 is zero, the whole thing is a demultiplexing problem instead of full CSD
            if(matrix.bottomLeftCorner(n,n).isZero(10e-14) && matrix.topRightCorner(n,n).isZero(10e-14))
            {
                COUT("Optimization: q2 is zero, only demultiplexing will be performed.");
                demultiplexing(matrix.topLeftCorner(n, n), matrix.bottomRightCorner(n,n), numberofbits-1);
                // The number of gates that would be necessary minus the number that is actually necessary to implement this unitary. (two unitaries one size smaller and one uniformly controlled rotation)
                int gatessaved = 3*std::pow(2, numberofbits-1) *(std::pow(2,numberofbits)-1) - ( 2*3*std::pow(2, numberofbits-2) *(std::pow(2,numberofbits-1)-1)+std::pow(2,numberofbits-2)*(std::pow(2,numberofbits)-2));
                for(int i = 0; i < gatessaved; i++)
                {
                    instructionlist.push_back(0);
                }
            }
            else
            {
            CSD(matrix, L0, L1, R0,R1,cc,ss);
            demultiplexing(R0,R1, numberofbits-1);
            multicontrolledY(ss,n);
            demultiplexing(L0,L1, numberofbits-1);

            }


        }
    }

    void CSD(complex_matrix U, complex_matrix &u1, complex_matrix &u2, complex_matrix &v1, complex_matrix &v2, complex_matrix &c, complex_matrix &s)
    {
            //Cosine sine decomposition
        // U = [q1, U01] = [u1    ][c  s][v1  ]
        //     [q2, U11] = [    u2][-s c][   v2]
        int n = U.rows();
        int m = U.cols();
        // std::cout << "u_rows " << n << std::endl;
        complex_matrix q1 = U.topLeftCorner(n/2,m/2);

        // std::cout << "U: " << U << std::endl;
        // std::cout << "q1: " << q1 << std::endl;
        // std::cout << "q2: " << q2 << std::endl;


        Eigen::BDCSVD<complex_matrix> svd;

        
        if(q1.rows() > 1 && q1.cols() > 1)
        {          
        svd.compute(q1, Eigen::ComputeThinU | Eigen::ComputeThinV); // possible because it's square anyway
        }

        // thinCSD: q1 = u1*c*v1.adjoint()
        //          q2 = u2*s*v1.adjoint()
        int p = q1.rows();
        complex_matrix z = Eigen::MatrixXd::Identity(p, p).colwise().reverse();
        c = z*svd.singularValues().asDiagonal()*z;
        u1 = svd.matrixU()*z;
        v1 = svd.matrixV()*z; // Same v as in matlab: u*s*v.adjoint() = q1
        complex_matrix q2 = U.bottomLeftCorner(p,p)*v1;      

        // std::cout << "c: " << c << std::endl;

        // std::cout << "q1 reconstructed:" << u1*c*v1.adjoint()<< std::endl;
        int k = 0;
        for(int j = 1; j < p; j++)
        {
            if(c(j,j).real() <= 0.70710678119)
            {
                k = j;
                // std::cout << "c(" << j << "): " << c(j,j) << std::endl;
            }
        }
        complex_matrix b = q2.block( 0,0, p, k+1);

        Eigen::HouseholderQR<complex_matrix> qr(p,p);
        // complex_matrix r = qr.matrixQR();
        qr.compute(b);
        u2 = qr.householderQ();
        s = u2.adjoint()*q2;
        // std::cout << "s: " << s << std::endl;
        // std::cout << "u2: " << u2 << std::endl;
        if(k < p-1)
        {
            DOUT("k is smaller than size of q1 = "<< p << ", adjustments will be made, k = " << k << std::endl);
            k = k+1;
            svd.compute(s.block(k, k, p-k, p-k));
            s.block(k, k, p-k, p-k) = svd.singularValues().asDiagonal();
            c.block(0,k, p,p-k) = c.block(0,k, p,p-k)*svd.matrixV();//.adjoint();
            u2.block(0,k, p,p-k) = u2.block(0,k, p,p-k)*svd.matrixU();
            v1.block(0,k, p,p-k) = v1.block(0,k, p,p-k)*svd.matrixV();//.adjoint();

            qr.compute(c.block(k,k, p-k,p-k));
            c.block(k,k,p-k,p-k) = qr.matrixQR();
            u1.block(0,k, p,p-k) = u1.block(0,k, p,p-k)*qr.householderQ(); 
            
            // std::cout << "z: " << ((complex_matrix) qr.householderQ()) << std::endl;
            // std::cout << "u1: " << u1 << std::endl;
            // std::cout << "u2: " << u2 << std::endl;
            // std::cout << "v1: " << v1 << std::endl;
            // std::cout << "s:" << s << std::endl;
        }
        for(int j = 0; j < p; j++)
        {
            if(c(j,j).real() < 0)
            {
                c(j,j) = -c(j,j);
                u1.col(j) = -u1.col(j);
            }
            if(s(j,j).real() < 0)
            {
                s(j,j) = -s(j,j);
                u2.col(j) = -u2.col(j);
            }
}
        if(!U.topLeftCorner(p,p).isApprox(u1*c*v1.adjoint(), 10e-8) || !U.bottomLeftCorner(p,p).isApprox(u2*s*v1.adjoint(), 10e-8))
        {
            if(U.topLeftCorner(p,p).isApprox(u1*c*v1.adjoint(), 10e-8))
            {
                COUT("q1 is correct");
            }
            else
            {
                COUT("q1 is not correct!");
                COUT("q1: " << U.topLeftCorner(p,p));
                COUT("reconstructed q1: " << u1*c*v1.adjoint());

            }
            if(U.bottomLeftCorner(p,p).isApprox(u2*s*v1.adjoint(), 10e-8))
            {
                COUT("q2 is correct");
            }
            else
            {
                COUT("q2 is not correct!");
                COUT("q2: " << U.bottomLeftCorner(p,p));
                COUT("reconstructed q2: " << u2*s*v1.adjoint());
            }
            // EOUT("thinCSD not correct!");
            // throw ql::exception("thinCSD of unitary '"+ name+"' not correct. Cannot be decomposed! Failed at matrix: \n"+to_string(q1) + " and matrix \n" + to_string(q2), false);
        }
        // std::cout << "reconstructed q1: " << u1*c*v1.adjoint() << std::endl;
        // std::cout << "reconstructed q2: " << u2*s*v1.adjoint() << std::endl;

        v2 = complex_matrix(p,p);
        v1.adjointInPlace(); // Us this instead of = v1.adjoint(0 to avoid aliasing issues)
        s = -s;
        for(int i = 0; i < n/2; i++)
        {
            if(std::abs(s(i,i)) > std::abs(c(i,i)))
            {
                complex_matrix tmp = u1.adjoint()*U.topRightCorner(p,p);
                v2.row(i) = tmp.row(i)/s(i,i);                
                // std::cout << "v2 (s): " << v2 << std::endl;
                // std::cout << "tmp (s): " << tmp << std::endl;
                // std::cout << "s(i,i): " << s(i,i) << std::endl;
                // std::cout << "i: " << i << std::endl;
            }
            else
            {
                complex_matrix tmp = u2.adjoint()*U.bottomRightCorner(p,p);
                v2.row(i) = tmp.row(i)/c(i,i);
                // std::cout << "v2 (c): " << v2 << std::endl;
                // std::cout << "tmp (c): " << tmp << std::endl;
                // std::cout << "c(i,i): " << c(i,i) << std::endl;
                // std::cout << "i: " << i << std::endl;
            }
        }
        // std::cout << "v2: " << v2 << std::endl;
        // U = [q1, U01] = [u1    ][c  s][v1  ]
        //     [q2, U11] = [    u2][-s c][   v2]

    
        complex_matrix tmp(n,m);
        tmp.topLeftCorner(p,p) = u1*c*v1;
        tmp.bottomLeftCorner(p,p) = -u2*s*v1;
        tmp.topRightCorner(p,p) = u1*s*v2;
        tmp.bottomRightCorner(p,p) = u2*c*v2;
        if(!tmp.isApprox(U, 10e-7))
        {
            COUT("CSD: reconstructed U");
            COUT(tmp);
            EOUT("CSD not correct!");
            throw ql::exception("CSD of unitary '"+ name+"' is wrong! Failed at matrix: \n"+to_string(tmp) + "\nwhich should be: \n" + to_string(U), false);

        }
        // std::cout << "CSD: reconstructed U" <<std::endl;
        // std::cout << tmp << std::endl;
    }

    std::vector<double> zyz_decomp(std::vector<std::complex<double>> matrix)
    {
         //TODO: make this efficient again
        ql::complex_t det = matrix[0]*matrix[3]-matrix[2]*matrix[1];
        //utils::print_vector(matrix, "matrix: " + std::to_string(matrix[0].real()) + ", " + std::to_string(matrix[1].real()) + ", "+ std::to_string(matrix[2].real()) + ", "+ std::to_string(matrix[3].real()) + ", "+ std::to_string(matrix[4].real()), "; ");

        double delta = atan2(det.imag(), det.real())/matrix.size();
        std::complex<double> com_two(0,1);
        std::complex<double> A = exp(-com_two*delta)*matrix[0];
        std::complex<double> B = exp(-com_two*delta)*matrix[1];
        double sw = sqrt(pow((double) B.imag(),2) + pow((double) B.real(),2) + pow((double) A.imag(),2));
        double wx = 0;
        double wy = 0;
        double wz = 0;
        if(sw > 0)
        {
        wx = B.imag()/sw;
        wy = B.real()/sw;
        wz = A.imag()/sw;
        }

        double t1 = atan2(A.imag(),A.real());
        double t2 = atan2(B.imag(), B.real());
        alpha = t1+t2;
        gamma = t1-t2;
        beta = 2*atan2(sw*sqrt(pow((double) wx,2)+pow((double) wy,2)),sqrt(pow((double) A.real(),2)+pow((wz*sw),2)));
        //instructionlist.push_back(delta);
        instructionlist.push_back(-gamma);
        instructionlist.push_back(-beta);
        instructionlist.push_back(-alpha);
        return instructionlist;
    }

    void zyz_decomp(complex_matrix matrix)
    {
        // std::cout << "zyz U: " << matrix << std::endl;
        ql::complex_t det = matrix(0,0)*matrix(1,1)-matrix(1,0)*matrix(0,1);
        //utils::print_vector(matrix, "matrix: " + std::to_string(matrix[0].real()) + ", " + std::to_string(matrix[1].real()) + ", "+ std::to_string(matrix[2].real()) + ", "+ std::to_string(matrix[3].real()) + ", "+ std::to_string(matrix[4].real()), "; ");

        double delta = atan2(det.imag(), det.real())/matrix.rows();
        std::complex<double> j(0,1); // 1j basically
        std::complex<double> A = exp(-j*delta)*matrix(0,0);
        std::complex<double> B = exp(-j*delta)*matrix(0,1); //to comply with the other y-gate definition?
        // std::cout << "A: " << A << std::endl;
        // std::cout << "B: " << B << std::endl;
        
        double sw = sqrt(pow((double) B.imag(),2) + pow((double) B.real(),2) + pow((double) A.imag(),2));
        double wx = 0;
        double wy = 0;
        double wz = 0;
        // std::cout << "sw: " << sw << std::endl;

        if(sw > 0)
        {
        wx = B.imag()/sw;
        wy = B.real()/sw;
        wz = A.imag()/sw;
        }
        // std::cout << "wx: " << wx << std::endl;
        // std::cout << "wy: " << wy << std::endl;
        // std::cout << "wz: " << wz << std::endl;


        double t1 = atan2(A.imag(),A.real());
        double t2 = atan2(B.imag(), B.real());
        alpha = t1+t2;
        gamma = t1-t2;
        beta = 2*atan2(sw*sqrt(pow((double) wx,2)+pow((double) wy,2)),sqrt(pow((double) A.real(),2)+pow((wz*sw),2)));
        //instructionlist.push_back(delta); //this is not used for the total decomposition
        // std::cout << "alpha: " << alpha << std::endl;
        // std::cout << "beta: " << beta << std::endl;
        // std::cout << "gamma: " << gamma << std::endl;        
        instructionlist.push_back(-gamma);
        instructionlist.push_back(-beta);
        instructionlist.push_back(-alpha);
    }


    void demultiplexing(complex_matrix U1, complex_matrix U2, int numberofcontrolbits)
    {
        // [U1 0 ]  = [V 0][D 0 ][W 0]
        // [0  U2]    [0 V][0 D*][0 W] 
        // std::cout << "Demultiplexing\nU1: " << U1 << std::endl;
        // std::cout << "U2: " << U2 << std::endl;
        if(U1 == U2)
        {
            if((int) U1.rows() == 2)
            {
                zyz_decomp(U1);
                COUT(" Optimization: Unitaries are equal, they are both: " << U1);
                //if U1 2x2, then the total gate is 4x4 = 2 qubit gates, which is a total of 3+3+2 rotation gates = 8 angles -> need to put 5 zeroes so the count is the same (and optimize them out later)
                for(int i = 0; i < 5; i++)
                {
                    instructionlist.push_back(0);
                }
                
            }
            else
            {

            COUT("Optimization: Unitaries are equal, skip one step in the recursion for unitaries of size: " << U1.rows() << " with this many control bits: " << numberofcontrolbits);
            decomp_function(U1, numberofcontrolbits);
            // The number of gates that would be necessary minus the number that is actually necessary to implement this unitary. 
            // One unitary one size smaller instead of two and a controlled rotation. (numberofcontrolbits = one less than the total number of qubits that this gate applies to)
            int gatessaved = 3*std::pow(2, numberofcontrolbits-1) *(std::pow(2,numberofcontrolbits)-1) + std::pow(2,numberofcontrolbits-1)*(std::pow(2,numberofcontrolbits+1)-2);
            for(int i = 0; i < gatessaved; i++)
            {
                instructionlist.push_back(0);
            }
            }
        }
        else
        {
            // std::cout << "U1*U2.adjoint(): " << U1*U2.adjoint() << std::endl;
            Eigen::ComplexEigenSolver<Eigen::MatrixXcd> eigslv(U1*U2.adjoint(), true); 
            complex_matrix d = eigslv.eigenvalues().asDiagonal();
            complex_matrix V = eigslv.eigenvectors();
            // std::cout << "d: " << d << std::endl;
            complex_matrix D = d.sqrt(); // Do this here to not get aliasing issues
            // std::cout << "D: " << D << std::endl;
            complex_matrix W = D*V.adjoint()*U2;
            // std::cout << "W: " << W << std::endl;
            // std::cout << "V: " << V << std::endl;
            if(!U1.isApprox(V*D*W, 10e-7) || !U2.isApprox(V*D.adjoint()*W, 10e-7))
            {
                COUT("Demultiplexing check U1: " << V*D*W);
                COUT("Demultiplexing check U2: " << V*D.adjoint()*W);
                EOUT("Demultiplexing not correct!");
                throw ql::exception("Demultiplexing of unitary '"+ name+"' not correct! Failed at matrix U1: \n"+to_string(U1)+ "and matrix U2: \n" +to_string(U2), false);
            }
            if(W.rows() == 2)
            {
                zyz_decomp(W);
            }
            else
            {
                decomp_function(W, std::log2(W.rows()));
            }
            multicontrolledZ(D, D.rows());
            if(V.rows() == 2)
            {
                zyz_decomp(V);
            }
            else
            {
                decomp_function(V, std::log2(V.rows()));
            }

            // std::cout << "Demultiplexing check U1: " << V*D*W << std::endl;
            // std::cout << "Demultiplexing check U2: " << V*D.adjoint()*W << std::endl;
        }
    }

    // returns M^k = (-1)^(b_(i-1)*g_(i-1)), where * is bitwise inner product, g = binary gray code, b = binary code.
    Eigen::MatrixXd genMk(int n)
    {
        //int b = n;
        //int g = n^(n>>1);
        Eigen::MatrixXd Mk(n,n);

        for(int i = 0; i < n; i++)
        {
            for(int j = 0; j < n ;j++)
            {
                Mk(i,j) =std::pow(-1, bitParity(i&(j^(j>>1))));
            }
        }
        return Mk;
    }

    int bitParity(int i)
    {
        if (i < 2 << 16)
        {
            i = (i >> 16) ^ i;
            i = (i >> 8) ^ i;
            i = (i >> 4) ^ i;
            i = (i >> 2) ^ i;
            i = (i >> 1) ^ i;
            return i % 2;
        }
        else
        {
            throw ql::exception("Bit parity number too big!", false);
        }
    }

    void multicontrolledY(complex_matrix ss, int halfthesizeofthematrix)
    {
        // std::cout << "ss: " << ss << std::endl;
        Eigen::VectorXd temp =  2*Eigen::asin(ss.diagonal().array()).real();
        Eigen::CompleteOrthogonalDecomposition<Eigen::MatrixXd> dec(genMk(halfthesizeofthematrix));
        Eigen::VectorXd tr = dec.solve(temp);
        // Eigen::VectorXd tr = (genMk(std::pow(2,numberofcontrolbits))).householderQr().solve(temp);
        // std::cout << "Mk: " << genMk(halfthesizeofthematrix) << std::endl;
        if(!temp.isApprox(genMk(halfthesizeofthematrix)*tr, 10e-7))
        {
                COUT("multicontrolledY check b: " << temp);
                COUT("multicontrolledY check A*x: " << genMk(halfthesizeofthematrix)*tr);
                EOUT("Multicontrolled Y not correct!");
                throw ql::exception("Demultiplexing of unitary '"+ name+"' not correct! Failed at demultiplexing of matrix ss: \n"  + to_string(ss), false);
        }
        for(int i = 0; i < halfthesizeofthematrix; i++)    
        {
            instructionlist.push_back(tr[i]);
        }
    }

    void multicontrolledZ(complex_matrix D, int halfthesizeofthematrix)
    {
        // std::cout << "D: " << D << std::endl;
        Eigen::VectorXd temp =  (2*Eigen::log(D.diagonal().array())/(std::complex<double>(0,1))).real();
        Eigen::CompleteOrthogonalDecomposition<Eigen::MatrixXd> dec(genMk(halfthesizeofthematrix));
        Eigen::VectorXd tr =dec.solve(temp);
        // Eigen::VectorXd tr = ((genMk(std::pow(2,numberofcontrolbits))).ColPivHouseholderQR().solve(temp));
        if(!temp.isApprox(genMk(halfthesizeofthematrix)*tr, 10e-7))
        {
                COUT("multicontrolledZ check b: " << temp);
                COUT("multicontrolledZ check A*x: " << genMk(halfthesizeofthematrix)*tr);
                EOUT("Multicontrolled Z not correct!");
                throw ql::exception("Demultiplexing of unitary '"+ name+"' not correct! Failed at demultiplexing of matrix D: \n"+ to_string(D), false);
        }
        
        for(int i = 0; i < halfthesizeofthematrix; i++)   
        {
            instructionlist.push_back(tr[i]);
        }
    }
    ~unitary()
    {
        // destroy unitary
        DOUT("destructing unitary: " << name);
    }
};


}

#endif // _UNITARY_H
