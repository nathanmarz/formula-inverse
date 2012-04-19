#ifndef MATH3D_H
#define MATH3D_H

#include <cmath>

#include <iostream>
 using namespace std; 


namespace Math3D {


#define PI 3.14159
    
// Define this to have Math3D.cpp generate a main which tests these classes
//#define TEST_MATH3D

// Define this to allow streaming output of vectors and matrices
// Automatically enabled by TEST_MATH3D
//#define OSTREAM_MATH3D

// definition of the scalar type
typedef float scalar_t;
// inline pass-throughs to various basic math functions
// written in this style to allow for easy substitution with more efficient versions
inline scalar_t SINE_FUNCTION (scalar_t x) 		{ return std::sin(x); }
inline scalar_t COSINE_FUNCTION (scalar_t x) 	{ return std::cos(x); }
inline scalar_t SQRT_FUNCTION (scalar_t x) 		{ return std::sqrt(x); }

// 4 element vector
class Vector4 {
public:
	Vector4 (void) {}
	Vector4 (scalar_t a, scalar_t b, scalar_t c, scalar_t d = 1)
	{ e[0]=a; e[1]=b; e[2]=c; e[3]=d; }
	
	// The int parameter is the number of elements to copy from initArray (3 or 4)
	explicit Vector4(scalar_t* initArray, int arraySize = 3)
	{ for (int i = 0;i<arraySize;++i) e[i] = initArray[i]; if (arraySize == 3) e[3] = 1; }
			
	// [] is to read, () is to write (const correctness)
	const scalar_t& operator[] (int i) const { return e[i]; }
	scalar_t& operator() (int i) { return e[i]; }
	
	// Provides access to the underlying array; useful for passing this class off to C APIs
	const scalar_t* readArray(void) { return e; }
	scalar_t* getArray(void) { return e; }
	
private:
	scalar_t e[4];
};

// 4 element matrix
class Matrix4 {
public:
	Matrix4 (void) {}
	
	// When defining matrices in C arrays, it is easiest to define them with
	// the column increasing fastest.  However, some APIs (OpenGL in particular) do this
	// backwards, hence the "constructor" from C matrices, or from OpenGL matrices.
	// Note that matrices are stored internally in OpenGL format.
	void C_Matrix (scalar_t* initArray)
	{ int i = 0; for (int y=0;y<4;++y) for (int x=0;x<4;++x) (*this)(x)[y] = initArray[i++]; }
	void OpenGL_Matrix (scalar_t* initArray)
	{ int i = 0; for (int x = 0; x < 4; ++x) for (int y=0;y<4;++y) (*this)(x)[y] = initArray[i++]; }
	
	// [] is to read, () is to write (const correctness)
	// m[x][y] or m(x)[y] is the correct form
	const scalar_t* operator[] (int i) const { return &e[i<<2]; }
	scalar_t* operator() (int i) { return &e[i<<2]; }
	
	// Low-level access to the array.
	const scalar_t* readArray (void) { return e; }
	scalar_t* getArray(void) { return e; }

	// Construct various matrices; REPLACES CURRENT CONTENTS OF THE MATRIX!
	// Written this way to work in-place and hence be somewhat more efficient
	void Identity (void) { for (int i=0;i<16;++i) e[i] = 0; e[0] = 1; e[5] = 1; e[10] = 1; e[15] = 1; }
	inline Matrix4& Rotation (scalar_t angle, Vector4 axis);
	inline Matrix4& Translation(const Vector4& translation);
	inline Matrix4& Scale (scalar_t x, scalar_t y, scalar_t z);
	inline Matrix4& BasisChange (const Vector4& v, const Vector4& n);
	inline Matrix4& BasisChange (const Vector4& u, const Vector4& v, const Vector4& n);
	inline Matrix4& ProjectionMatrix (bool perspective, scalar_t l, scalar_t r, scalar_t t, scalar_t b, scalar_t n, scalar_t f);
	
private:
	scalar_t e[16];
};

// Scalar operations

// Returns false if there are 0 solutions
inline bool SolveQuadratic (scalar_t a, scalar_t b, scalar_t c, scalar_t* x1, scalar_t* x2);

// Vector operations
inline bool operator== (const Vector4&, const Vector4&);
inline bool operator< (const Vector4&, const Vector4&);

inline Vector4 operator- (const Vector4&);
inline Vector4 operator* (const Vector4&, scalar_t);
inline Vector4 operator* (scalar_t, const Vector4&);
inline Vector4& operator*= (Vector4&, scalar_t);
inline Vector4 operator/ (const Vector4&, scalar_t);
inline Vector4& operator/= (Vector4&, scalar_t);

inline Vector4 operator+ (const Vector4&, const Vector4&);
inline Vector4& operator+= (Vector4&, const Vector4&);
inline Vector4 operator- (const Vector4&, const Vector4&);
inline Vector4& operator-= (Vector4&, const Vector4&);

// X3 is the 3 element version of a function, X4 is four element
inline scalar_t LengthSquared3 (const Vector4&);
inline scalar_t LengthSquared4 (const Vector4&);
inline scalar_t Length3 (const Vector4&);
inline scalar_t Length4 (const Vector4&);
//returns radians 
inline scalar_t Angle3(const Vector4 &v1, const Vector4& v2); 
inline Vector4 Normalize3 (const Vector4&);
inline Vector4 Normalize4 (const Vector4&);
inline scalar_t DotProduct3 (const Vector4&, const Vector4&);
inline scalar_t DotProduct4 (const Vector4&, const Vector4&);
// Cross product is only defined for 3 elements
inline Vector4 CrossProduct (const Vector4&, const Vector4&);

inline Vector4 operator* (const Matrix4&, const Vector4&);

// Matrix operations
inline bool operator== (const Matrix4&, const Matrix4&);
inline bool operator< (const Matrix4&, const Matrix4&);

inline Matrix4 operator* (const Matrix4&, const Matrix4&);

inline Matrix4 Transpose (const Matrix4&);
scalar_t Determinant (const Matrix4&);
Matrix4 Adjoint (const Matrix4&);
//Matrix4 Inverse (const Matrix4&);

// Inline implementations follow
inline bool SolveQuadratic (scalar_t a, scalar_t b, scalar_t c, scalar_t* x1, scalar_t* x2) {
	// If a == 0, solve a linear equation
	if (a == 0) {
		if (b == 0) return false;
		*x1 = c / b;
		*x2 = *x1;
		return true;
	} else {
		scalar_t det = b * b - 4 * a * c;
		if (det < 0) return false;
		det = SQRT_FUNCTION(det) / (2 * a);
		scalar_t prefix = -b / (2*a);
		*x1 = prefix + det;
		*x2 = prefix - det;
		return true;
	}
}
 
 
 
inline scalar_t Angle3(const Vector4 &v1, const Vector4& v2) {
    
    
    float toacos = DotProduct3(v1,v2)/(Length3(v1)*Length3(v2));

    
    //cerr<<toacos<<endl;
    if(toacos>=1) return 0;
    if(toacos<=-1) return PI;
    float ret = acos(DotProduct3(v1,v2)/(Length3(v1)*Length3(v2)));
    //if(ret*0!=0) {
    //    exit(1);
    // }
    return ret;
}

inline bool operator== (const Vector4& v1, const Vector4& v2) 
{ return (v1[0]==v2[0]&&v1[1]==v2[1]&&v1[2]==v2[2]&&v1[3]==v2[3]); }

inline bool operator< (const Vector4& v1, const Vector4& v2) {
	for (int i=0;i<4;++i) 
		if (v1[i] < v2[i]) return true;
		else if (v1[i] > v2[i]) return false;
	return false;
}

inline Vector4 operator- (const Vector4& v) 
{ return Vector4(-v[0], -v[1], -v[2], -v[3]); }

inline Vector4 operator* (const Vector4& v, scalar_t k)
{ return Vector4(k*v[0], k*v[1], k*v[2], k*v[3]); }

inline Vector4 operator* (scalar_t k, const Vector4& v)
{ return v * k; }

inline Vector4& operator*= (Vector4& v, scalar_t k)
{ for (int i=0;i<4;++i) v(i) *= k; return v; }

inline Vector4 operator/ (const Vector4& v, scalar_t k)
{ return Vector4(v[0]/k, v[1]/k, v[2]/k, v[3]/k); }

inline Vector4& operator/= (Vector4& v, scalar_t k)
{ for (int i=0;i<4;++i) v(i) /= k; return v; }

inline scalar_t LengthSquared3 (const Vector4& v)
{ return DotProduct3(v,v); }
inline scalar_t LengthSquared4 (const Vector4& v)
{ return DotProduct4(v,v); }

inline scalar_t Length3 (const Vector4& v)
{ return SQRT_FUNCTION(LengthSquared3(v)); }
inline scalar_t Length4 (const Vector4& v)
{ return SQRT_FUNCTION(LengthSquared4(v)); }

inline Vector4 Normalize3 (const Vector4& v)
{	Vector4 retVal = v / Length3(v); retVal(3) = 1; return retVal; }
inline Vector4 Normalize4 (const Vector4& v)
{	return v / Length4(v); }

inline Vector4 operator+ (const Vector4& v1, const Vector4& v2)
{ return Vector4(v1[0]+v2[0], v1[1]+v2[1], v1[2]+v2[2], v1[3]+v2[3]); }

inline Vector4& operator+= (Vector4& v1, const Vector4& v2)
{ for (int i=0;i<4;++i) v1(i) += v2[i]; return v1; }

inline Vector4 operator- (const Vector4& v1, const Vector4& v2)
{ return Vector4(v1[0]-v2[0], v1[1]-v2[1], v1[2]-v2[2], v1[3]-v2[3]); }

inline Vector4& operator-= (Vector4& v1, const Vector4& v2)
{ for (int i=0;i<4;++i) v1(i) -= v2[i]; return v1; }

inline scalar_t DotProduct3 (const Vector4& v1, const Vector4& v2)
{ return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]; }

inline scalar_t DotProduct4 (const Vector4& v1, const Vector4& v2)
{ return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2] + v1[3]*v2[3]; }

inline Vector4 CrossProduct (const Vector4& v1, const Vector4& v2) {
return Vector4( 	 v1[1] * v2[2] - v1[2] * v2[1]
					,v2[0] * v1[2] - v2[2] * v1[0]
					,v1[0] * v2[1] - v1[1] * v2[0]
					,1);
}

inline Vector4 operator* (const Matrix4& m, const Vector4& v) {
	return Vector4( v[0]*m[0][0] + v[1]*m[1][0] + v[2]*m[2][0] + v[3]*m[3][0],
					v[0]*m[0][1] + v[1]*m[1][1] + v[2]*m[2][1] + v[3]*m[3][1],
					v[0]*m[0][2] + v[1]*m[1][2] + v[2]*m[2][2] + v[3]*m[3][2],
					v[0]*m[0][3] + v[1]*m[1][3] + v[2]*m[2][3] + v[3]*m[3][3]);
					
}

inline bool operator== (const Matrix4& m1, const Matrix4& m2) {
	for (int x=0;x<4;++x) for (int y=0;y<4;++y) 
		if (m1[x][y] != m2[x][y]) return false;
	return true;		
}

inline bool operator< (const Matrix4& m1, const Matrix4& m2) {
	for (int x=0;x<4;++x) for (int y=0;y<4;++y) 
		if (m1[x][y] < m2[x][y]) return true;
		else if (m1[x][y] > m2[x][y]) return false;
	return false;		
}

inline Matrix4 operator* (const Matrix4& m1, const Matrix4& m2) {
	Matrix4 retVal;
	for (int x=0;x<4;++x) for (int y=0;y<4;++y) {
		retVal(x)[y] = 0;
		for (int i=0;i<4;++i) retVal(x)[y] += m1[i][y] * m2[x][i];
	}
	return retVal;
}

inline Matrix4 Transpose (const Matrix4& m) {
	Matrix4 retVal;
	for (int x=0;x<4;++x) for (int y=0;y<4;++y) 
		retVal(x)[y] = m[y][x];
	return retVal;
}

inline Matrix4& Matrix4::Rotation (scalar_t angle, Vector4 axis) {
	scalar_t c = COSINE_FUNCTION(angle);
	scalar_t s = SINE_FUNCTION(angle);
	// One minus c (short name for legibility of formulai)
	scalar_t omc = (1 - c);
	
	if (LengthSquared3(axis) != 1) axis = Normalize3(axis);
	
	scalar_t x = axis[0];
	scalar_t y = axis[1];
	scalar_t z = axis[2];	
	scalar_t xs = x * s;
	scalar_t ys = y * s;
	scalar_t zs = z * s;
	scalar_t xyomc = x * y * omc;
	scalar_t xzomc = x * z * omc;
	scalar_t yzomc = y * z * omc;
	
	e[0] = x*x*omc + c;
	e[1] = xyomc + zs;
	e[2] = xzomc - ys;
	e[3] = 0;
	
	e[4] = xyomc - zs;
	e[5] = y*y*omc + c;
	e[6] = yzomc + xs;
	e[7] = 0;
	
	e[8] = xzomc + ys;
	e[9] = yzomc - xs;
	e[10] = z*z*omc + c;
	e[11] = 0;
	
	e[12] = 0;
	e[13] = 0;
	e[14] = 0;
	e[15] = 1;
	
	return *this;	
}

inline Matrix4& Matrix4::Translation(const Vector4& translation) {
	Identity();
	e[12] = translation[0];
	e[13] = translation[1];
	e[14] = translation[2];
	return *this;
}

inline Matrix4& Matrix4::Scale (scalar_t x, scalar_t y, scalar_t z) {
	Identity();
	e[0] = x;
	e[5] = y;
	e[10] = z;
	return *this;
}

inline Matrix4& Matrix4::BasisChange (const Vector4& u, const Vector4& v, const Vector4& n) {
	e[0] = u[0];
	e[1] = v[0];
	e[2] = n[0];
	e[3] = 0;
	
	e[4] = u[1];
	e[5] = v[1];
	e[6] = n[1];
	e[7] = 0;
	
	e[8] = u[2];
	e[9] = v[2];
	e[10] = n[2];
	e[11] = 0;
	
	e[12] = 0;
	e[13] = 0;
	e[14] = 0;
	e[15] = 1;
	
	return *this;
}

inline Matrix4& Matrix4::BasisChange (const Vector4& v, const Vector4& n) {
	Vector4 u = CrossProduct(v,n);
	return BasisChange (u, v, n);
}

inline Matrix4& Matrix4::ProjectionMatrix (bool perspective, 	scalar_t left_plane, scalar_t right_plane, 
																scalar_t top_plane, scalar_t bottom_plane, 
																scalar_t near_plane, scalar_t far_plane)
{
	scalar_t A = (right_plane + left_plane) / (right_plane - left_plane);
	scalar_t B = (top_plane + bottom_plane) / (top_plane - bottom_plane);
	scalar_t C = (far_plane + near_plane) / (far_plane - near_plane);
	
	Identity();
	if (perspective) {
		e[0] = 2 * near_plane / (right_plane - left_plane);
		e[5] = 2 * near_plane / (top_plane - bottom_plane);
		e[8] = A;
		e[9] = B;
		e[10] = C;
		e[11] = -1;
		e[14] =	2 * far_plane * near_plane / (far_plane - near_plane);	
	} else {
		e[0] = 2 / (right_plane - left_plane);
		e[5] = 2 / (top_plane - bottom_plane);
		e[10] = -2 / (far_plane - near_plane);
		e[12] = A;
		e[13] =  B;
		e[14] = C;
	}
	return *this;
}


} // close namespace

void PrintVector(const Math3D::Vector4& v);
void PrintMatrix(const Math3D::Matrix4 &m);
/*
// Streaming support
std::ostream& operator<< (std::ostream& os, const Math3D::Vector4& v) {
	os << '[';
	for (int i=0; i<4; ++i)
		os << ' ' << v[i];
	return os << ']';
}

std::ostream& operator<< (std::ostream& os, const Math3D::Matrix4& m) {
	for (int y=0; y<4; ++y) {
		os << '[';
		for (int x=0;x<4;++x)
			os << ' ' << m[x][y];
		os << " ]" << std::endl;
	}
	return os;
}
*/


#endif
