#include <Math3D.h>
#include <iostream>
//#include "gmMat4.h"

using namespace std;
using namespace Math3D;

void PrintVector(const Math3D::Vector4& v) {
    cerr<<"[";
    for (int i=0; i<4; ++i)
		cerr << ' ' << v[i];
	cerr << ']'<<endl;

}

void PrintMatrix(const Math3D::Matrix4 &m) {
	for (int y=0; y<4; ++y) {
		cerr << '[';
		for (int x=0;x<4;++x)
			cerr << ' ' << m[x][y];
		cerr << " ]" << std::endl;
	}
    cerr<<endl;


}

/*
Matrix4 Math3D::Inverse (const Matrix4& mat) {
    gmMatrix4 m;
    for(int i=0;i<4;i++) {
        for(int j=0;j<4;j++) {
            m[i][j] = mat[j][i];
        }
    }
    gmMatrix4 inv = m.inverse();
    Matrix4 ret;
    for(int i=0;i<4;i++) {
        for(int j=0;j<4;j++) {
            ret(j)[i] = inv[i][j];
        }
    }
    //PrintMatrix(ret*mat);
    return ret;
}
*/
