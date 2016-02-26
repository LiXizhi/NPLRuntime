#pragma once

namespace ParaEngine
{

	/**
	*	3x3 matrix.
	*	DirectX-compliant, ie row-column order, ie m[Row][Col].
	*	Same as:
	*	m11  m12  m13  first row.
	*	m21  m22  m23  second row.
	*	m31  m32  m33  third row.
	*	Stored in memory as m11 m12 m13 m21...
	*
	*	Multiplication rules:
	*
	*	[x'y'z'] = [xyz][M]
	*
	*	x' = x*m11 + y*m21 + z*m31
	*	y' = x*m12 + y*m22 + z*m32
	*	z' = x*m13 + y*m23 + z*m33
	*
	*/
	class Matrix3x3
	{
	public:
		Matrix3x3(void);
		~Matrix3x3(void);

		//! Empty constructor
		//! Constructor from 9 values
		Matrix3x3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22)
		{
			m[0][0] = m00;	m[0][1] = m01;	m[0][2] = m02;
			m[1][0] = m10;	m[1][1] = m11;	m[1][2] = m12;
			m[2][0] = m20;	m[2][1] = m21;	m[2][2] = m22;
		}
		Matrix3x3(const D3DXMATRIX& mat4x4);
		//! Copy constructor
		Matrix3x3(const Matrix3x3& mat);
		
		//! Assign values
		void			Set(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22)
		{
			m[0][0] = m00;	m[0][1] = m01;	m[0][2] = m02;
			m[1][0] = m10;	m[1][1] = m11;	m[1][2] = m12;
			m[2][0] = m20;	m[2][1] = m21;	m[2][2] = m22;
		}

		//! Sets the scale from a D3DXVECTOR3. The point is put on the diagonal.
		void			SetScale(const D3DXVECTOR3& p)					{ m[0][0] = p.x;	m[1][1] = p.y;	m[2][2] = p.z;	}

		//! Sets the scale from floats. Values are put on the diagonal.
		void			SetScale(float sx, float sy, float sz)		{ m[0][0] = sx;		m[1][1] = sy;	m[2][2] = sz;	}

		//! Scales from a D3DXVECTOR3. Each row is multiplied by a component.
		void			Scale(const D3DXVECTOR3& p)
		{
			m[0][0] *= p.x;	m[0][1] *= p.x;	m[0][2] *= p.x;
			m[1][0] *= p.y;	m[1][1] *= p.y;	m[1][2] *= p.y;
			m[2][0] *= p.z;	m[2][1] *= p.z;	m[2][2] *= p.z;
		}

		//! Scales from floats. Each row is multiplied by a value.
		void			Scale(float sx, float sy, float sz)
		{
			m[0][0] *= sx;	m[0][1] *= sx;	m[0][2] *= sx;
			m[1][0] *= sy;	m[1][1] *= sy;	m[1][2] *= sy;
			m[2][0] *= sz;	m[2][1] *= sz;	m[2][2] *= sz;
		}

		//! Copy from a Matrix3x3
		void			Copy(const Matrix3x3& source);

		// Row-column access
		//! Returns a row.
		void			GetRow(const DWORD r, D3DXVECTOR3& p)	const	{ p.x = m[r][0];	p.y = m[r][1];	p.z = m[r][2];	}
		//! Returns a row.
		const D3DXVECTOR3&	GetRow(const DWORD r)				const	{ return *(const D3DXVECTOR3*)&m[r][0];	}
		//! Returns a row.
		D3DXVECTOR3&			GetRow(const DWORD r)						{ return *(D3DXVECTOR3*)&m[r][0];			}
		//! Sets a row.
		void			SetRow(const DWORD r, const D3DXVECTOR3& p)		{ m[r][0] = p.x;	m[r][1] = p.y;	m[r][2] = p.z;	}
		//! Returns a column.
		void			GetCol(const DWORD c, D3DXVECTOR3& p)	const	{ p.x = m[0][c];	p.y = m[1][c];	p.z = m[2][c];	}
		//! Sets a column.
		void			SetCol(const DWORD c, const D3DXVECTOR3& p)		{ m[0][c] = p.x;	m[1][c] = p.y;	m[2][c] = p.z;	}

		//! Computes the trace. The trace is the sum of the 3 diagonal components.
		float			Trace()					const				{ return m[0][0] + m[1][1] + m[2][2];				}
		//! Clears the matrix.
		void			Zero();
		//! Sets the identity matrix.
		void			Identity()									{ Zero(); m[0][0] = m[1][1] = m[2][2] = 1.0f; 		}
		//! Checks for identity
		bool			IsIdentity()			const;
		
		//! Makes a skew-symmetric matrix (a.k.a. Star(*) Matrix)
		//!	[  0.0  -a.z   a.y ]
		//!	[  a.z   0.0  -a.x ]
		//!	[ -a.y   a.x   0.0 ]
		//! This is also called a "cross matrix" since for any vectors A and B,
		//! A^B = Skew(A) * B = - B * Skew(A);
		void			SkewSymmetric(const D3DXVECTOR3& a)
		{
			m[0][0] = 0.0f;
			m[0][1] = -a.z;
			m[0][2] = a.y;

			m[1][0] = a.z;
			m[1][1] = 0.0f;
			m[1][2] = -a.x;

			m[2][0] = -a.y;
			m[2][1] = a.x;
			m[2][2] = 0.0f;
		}

		//! Negates the matrix
		void			Neg()
		{
			m[0][0] = -m[0][0];	m[0][1] = -m[0][1];	m[0][2] = -m[0][2];
			m[1][0] = -m[1][0];	m[1][1] = -m[1][1];	m[1][2] = -m[1][2];
			m[2][0] = -m[2][0];	m[2][1] = -m[2][1];	m[2][2] = -m[2][2];
		}

		//! Neg from another matrix
		void			Neg(const Matrix3x3& mat)
		{
			m[0][0] = -mat.m[0][0];	m[0][1] = -mat.m[0][1];	m[0][2] = -mat.m[0][2];
			m[1][0] = -mat.m[1][0];	m[1][1] = -mat.m[1][1];	m[1][2] = -mat.m[1][2];
			m[2][0] = -mat.m[2][0];	m[2][1] = -mat.m[2][1];	m[2][2] = -mat.m[2][2];
		}

		//! Add another matrix
		void			Add(const Matrix3x3& mat)
		{
			m[0][0] += mat.m[0][0];	m[0][1] += mat.m[0][1];	m[0][2] += mat.m[0][2];
			m[1][0] += mat.m[1][0];	m[1][1] += mat.m[1][1];	m[1][2] += mat.m[1][2];
			m[2][0] += mat.m[2][0];	m[2][1] += mat.m[2][1];	m[2][2] += mat.m[2][2];
		}

		//! Sub another matrix
		void			Sub(const Matrix3x3& mat)
		{
			m[0][0] -= mat.m[0][0];	m[0][1]	-= mat.m[0][1];	m[0][2] -= mat.m[0][2];
			m[1][0] -= mat.m[1][0];	m[1][1] -= mat.m[1][1];	m[1][2] -= mat.m[1][2];
			m[2][0] -= mat.m[2][0];	m[2][1] -= mat.m[2][1];	m[2][2] -= mat.m[2][2];
		}
		//! Mac
		void			Mac(const Matrix3x3& a, const Matrix3x3& b, float s)
		{
			m[0][0] = a.m[0][0] + b.m[0][0] * s;
			m[0][1] = a.m[0][1] + b.m[0][1] * s;
			m[0][2] = a.m[0][2] + b.m[0][2] * s;

			m[1][0] = a.m[1][0] + b.m[1][0] * s;
			m[1][1] = a.m[1][1] + b.m[1][1] * s;
			m[1][2] = a.m[1][2] + b.m[1][2] * s;

			m[2][0] = a.m[2][0] + b.m[2][0] * s;
			m[2][1] = a.m[2][1] + b.m[2][1] * s;
			m[2][2] = a.m[2][2] + b.m[2][2] * s;
		}
		//! Mac
		void			Mac(const Matrix3x3& a, float s)
		{
			m[0][0] += a.m[0][0] * s;	m[0][1] += a.m[0][1] * s;	m[0][2] += a.m[0][2] * s;
			m[1][0] += a.m[1][0] * s;	m[1][1] += a.m[1][1] * s;	m[1][2] += a.m[1][2] * s;
			m[2][0] += a.m[2][0] * s;	m[2][1] += a.m[2][1] * s;	m[2][2] += a.m[2][2] * s;
		}

		//! this = A * s
		void			Mult(const Matrix3x3& a, float s)
		{
			m[0][0] = a.m[0][0] * s;	m[0][1] = a.m[0][1] * s;	m[0][2] = a.m[0][2] * s;
			m[1][0] = a.m[1][0] * s;	m[1][1] = a.m[1][1] * s;	m[1][2] = a.m[1][2] * s;
			m[2][0] = a.m[2][0] * s;	m[2][1] = a.m[2][1] * s;	m[2][2] = a.m[2][2] * s;
		}

		void			Add(const Matrix3x3& a, const Matrix3x3& b)
		{
			m[0][0] = a.m[0][0] + b.m[0][0];	m[0][1] = a.m[0][1] + b.m[0][1];	m[0][2] = a.m[0][2] + b.m[0][2];
			m[1][0] = a.m[1][0] + b.m[1][0];	m[1][1] = a.m[1][1] + b.m[1][1];	m[1][2] = a.m[1][2] + b.m[1][2];
			m[2][0] = a.m[2][0] + b.m[2][0];	m[2][1] = a.m[2][1] + b.m[2][1];	m[2][2] = a.m[2][2] + b.m[2][2];
		}

		void			Sub(const Matrix3x3& a, const Matrix3x3& b)
		{
			m[0][0] = a.m[0][0] - b.m[0][0];	m[0][1] = a.m[0][1] - b.m[0][1];	m[0][2] = a.m[0][2] - b.m[0][2];
			m[1][0] = a.m[1][0] - b.m[1][0];	m[1][1] = a.m[1][1] - b.m[1][1];	m[1][2] = a.m[1][2] - b.m[1][2];
			m[2][0] = a.m[2][0] - b.m[2][0];	m[2][1] = a.m[2][1] - b.m[2][1];	m[2][2] = a.m[2][2] - b.m[2][2];
		}

		//! this = a * b
		void			Mult(const Matrix3x3& a, const Matrix3x3& b)
		{
			m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] + a.m[0][2] * b.m[2][0];
			m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[2][1];
			m[0][2] = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] + a.m[0][2] * b.m[2][2];
			m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[1][2] * b.m[2][0];
			m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[2][1];
			m[1][2] = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[1][2] * b.m[2][2];
			m[2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] + a.m[2][2] * b.m[2][0];
			m[2][1] = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[2][1];
			m[2][2] = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] + a.m[2][2] * b.m[2][2];
		}

		//! this = transpose(a) * b
		void			MultAtB(const Matrix3x3& a, const Matrix3x3& b)
		{
			m[0][0] = a.m[0][0] * b.m[0][0] + a.m[1][0] * b.m[1][0] + a.m[2][0] * b.m[2][0];
			m[0][1] = a.m[0][0] * b.m[0][1] + a.m[1][0] * b.m[1][1] + a.m[2][0] * b.m[2][1];
			m[0][2] = a.m[0][0] * b.m[0][2] + a.m[1][0] * b.m[1][2] + a.m[2][0] * b.m[2][2];
			m[1][0] = a.m[0][1] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[2][1] * b.m[2][0];
			m[1][1] = a.m[0][1] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[2][1] * b.m[2][1];
			m[1][2] = a.m[0][1] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[2][1] * b.m[2][2];
			m[2][0] = a.m[0][2] * b.m[0][0] + a.m[1][2] * b.m[1][0] + a.m[2][2] * b.m[2][0];
			m[2][1] = a.m[0][2] * b.m[0][1] + a.m[1][2] * b.m[1][1] + a.m[2][2] * b.m[2][1];
			m[2][2] = a.m[0][2] * b.m[0][2] + a.m[1][2] * b.m[1][2] + a.m[2][2] * b.m[2][2];
		}

		//! this = a * transpose(b)
		void			MultABt(const Matrix3x3& a, const Matrix3x3& b)
		{
			m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[0][1] + a.m[0][2] * b.m[0][2];
			m[0][1] = a.m[0][0] * b.m[1][0] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[1][2];
			m[0][2] = a.m[0][0] * b.m[2][0] + a.m[0][1] * b.m[2][1] + a.m[0][2] * b.m[2][2];
			m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[0][1] + a.m[1][2] * b.m[0][2];
			m[1][1] = a.m[1][0] * b.m[1][0] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[1][2];
			m[1][2] = a.m[1][0] * b.m[2][0] + a.m[1][1] * b.m[2][1] + a.m[1][2] * b.m[2][2];
			m[2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[0][1] + a.m[2][2] * b.m[0][2];
			m[2][1] = a.m[2][0] * b.m[1][0] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[1][2];
			m[2][2] = a.m[2][0] * b.m[2][0] + a.m[2][1] * b.m[2][1] + a.m[2][2] * b.m[2][2];
		}

		//! Makes a rotation matrix mapping vector "from" to vector "to".
		Matrix3x3&		FromTo(const D3DXVECTOR3& from, const D3DXVECTOR3& to);

		//! Set a rotation matrix around the X axis.
		//!		 1		0		0
		//!	RX = 0		cx		sx
		//!		 0		-sx		cx
		void				RotX(float angle)	{ float Cos = cosf(angle), Sin = sinf(angle); Identity(); m[1][1] = m[2][2] = Cos; m[2][1] = -Sin;	m[1][2] = Sin;	}

		//! Set a rotation matrix around the Y axis.
		//!		 cy		0		-sy
		//!	RY = 0		1		0
		//!		 sy		0		cy
		void				RotY(float angle)	{ float Cos = cosf(angle), Sin = sinf(angle); Identity(); m[0][0] = m[2][2] = Cos; m[2][0] = Sin;	m[0][2] = -Sin;	}

		//! Set a rotation matrix around the Z axis.
		//!		 cz		sz		0
		//!	RZ = -sz	cz		0
		//!		 0		0		1
		void				RotZ(float angle)	{ float Cos = cosf(angle), Sin = sinf(angle); Identity(); m[0][0] = m[1][1] = Cos; m[1][0] = -Sin;	m[0][1] = Sin;	}

		//!			cy		sx.sy		-sy.cx
		//!	RY.RX	0		cx			sx
		//!			sy		-sx.cy		cx.cy
		void			RotYX(float y, float x);

		//! Make a rotation matrix about an arbitrary axis
		Matrix3x3&		Rot(float angle, const D3DXVECTOR3& axis);

		//! Transpose the matrix.
		void			Transpose();

		//! this = Transpose(a)
		void			Transpose(const Matrix3x3& a)
		{
			m[0][0] = a.m[0][0];	m[0][1] = a.m[1][0];	m[0][2] = a.m[2][0];
			m[1][0] = a.m[0][1];	m[1][1] = a.m[1][1];	m[1][2] = a.m[2][1];
			m[2][0] = a.m[0][2];	m[2][1] = a.m[1][2];	m[2][2] = a.m[2][2];
		}

		//! Compute the determinant of the matrix. We use the rule of Sarrus.
		float			Determinant()					const
		{
			return (m[0][0]*m[1][1]*m[2][2] + m[0][1]*m[1][2]*m[2][0] + m[0][2]*m[1][0]*m[2][1])
				-  (m[2][0]*m[1][1]*m[0][2] + m[2][1]*m[1][2]*m[0][0] + m[2][2]*m[1][0]*m[0][1]);
		}
		/*
		//! Compute a cofactor. Used for matrix inversion.
		float			CoFactor(ubyte row, ubyte column)	const
		{
		static sdword gIndex[3+2] = { 0, 1, 2, 0, 1 };
		return	(m[gIndex[row+1]][gIndex[column+1]]*m[gIndex[row+2]][gIndex[column+2]] - m[gIndex[row+2]][gIndex[column+1]]*m[gIndex[row+1]][gIndex[column+2]]);
		}
		*/
		//! Invert the matrix. Determinant must be different from zero, else matrix can't be inverted.
		Matrix3x3&		Invert()
		{
			float Det = Determinant();	// Must be !=0
			float OneOverDet = 1.0f / Det;

			Matrix3x3 Temp;
			Temp.m[0][0] = +(m[1][1] * m[2][2] - m[2][1] * m[1][2]) * OneOverDet;
			Temp.m[1][0] = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]) * OneOverDet;
			Temp.m[2][0] = +(m[1][0] * m[2][1] - m[2][0] * m[1][1]) * OneOverDet;
			Temp.m[0][1] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]) * OneOverDet;
			Temp.m[1][1] = +(m[0][0] * m[2][2] - m[2][0] * m[0][2]) * OneOverDet;
			Temp.m[2][1] = -(m[0][0] * m[2][1] - m[2][0] * m[0][1]) * OneOverDet;
			Temp.m[0][2] = +(m[0][1] * m[1][2] - m[1][1] * m[0][2]) * OneOverDet;
			Temp.m[1][2] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]) * OneOverDet;
			Temp.m[2][2] = +(m[0][0] * m[1][1] - m[1][0] * m[0][1]) * OneOverDet;

			*this = Temp;

			return	*this;
		}

		Matrix3x3&		Normalize();

		//! this = exp(a)
		Matrix3x3&		Exp(const Matrix3x3& a);

		void FromQuat(const D3DXQUATERNION &q);
		void FromQuatL2(const D3DXQUATERNION&q, float l2);

		// Arithmetic operators
		//! Operator for Matrix3x3 Plus = Matrix3x3 + Matrix3x3;
		Matrix3x3		operator+(const Matrix3x3& mat)	const
		{
			return Matrix3x3(
				m[0][0] + mat.m[0][0],	m[0][1] + mat.m[0][1],	m[0][2] + mat.m[0][2],
				m[1][0] + mat.m[1][0],	m[1][1] + mat.m[1][1],	m[1][2] + mat.m[1][2],
				m[2][0] + mat.m[2][0],	m[2][1] + mat.m[2][1],	m[2][2] + mat.m[2][2]);
		}

		//! Operator for Matrix3x3 Minus = Matrix3x3 - Matrix3x3;
		Matrix3x3		operator-(const Matrix3x3& mat)	const
		{
			return Matrix3x3(
				m[0][0] - mat.m[0][0],	m[0][1] - mat.m[0][1],	m[0][2] - mat.m[0][2],
				m[1][0] - mat.m[1][0],	m[1][1] - mat.m[1][1],	m[1][2] - mat.m[1][2],
				m[2][0] - mat.m[2][0],	m[2][1] - mat.m[2][1],	m[2][2] - mat.m[2][2]);
		}

		//! Operator for Matrix3x3 Mul = Matrix3x3 * Matrix3x3;
		Matrix3x3		operator*(const Matrix3x3& mat)	const
		{
			return Matrix3x3(
				m[0][0]*mat.m[0][0] + m[0][1]*mat.m[1][0] + m[0][2]*mat.m[2][0],
				m[0][0]*mat.m[0][1] + m[0][1]*mat.m[1][1] + m[0][2]*mat.m[2][1],
				m[0][0]*mat.m[0][2] + m[0][1]*mat.m[1][2] + m[0][2]*mat.m[2][2],

				m[1][0]*mat.m[0][0] + m[1][1]*mat.m[1][0] + m[1][2]*mat.m[2][0],
				m[1][0]*mat.m[0][1] + m[1][1]*mat.m[1][1] + m[1][2]*mat.m[2][1],
				m[1][0]*mat.m[0][2] + m[1][1]*mat.m[1][2] + m[1][2]*mat.m[2][2],

				m[2][0]*mat.m[0][0] + m[2][1]*mat.m[1][0] + m[2][2]*mat.m[2][0],
				m[2][0]*mat.m[0][1] + m[2][1]*mat.m[1][1] + m[2][2]*mat.m[2][1],
				m[2][0]*mat.m[0][2] + m[2][1]*mat.m[1][2] + m[2][2]*mat.m[2][2]);
		}

		//! Operator for D3DXVECTOR3 Mul = Matrix3x3 * D3DXVECTOR3;
		D3DXVECTOR3			operator*(const D3DXVECTOR3& v)		const;

		//! Operator for Matrix3x3 Mul = Matrix3x3 * float;
		Matrix3x3		operator*(float s)				const
		{
			return Matrix3x3(
				m[0][0]*s,	m[0][1]*s,	m[0][2]*s,
				m[1][0]*s,	m[1][1]*s,	m[1][2]*s,
				m[2][0]*s,	m[2][1]*s,	m[2][2]*s);
		}

		//! Operator for Matrix3x3 Mul = float * Matrix3x3;
		friend Matrix3x3 operator*(float s, const Matrix3x3& mat)
		{
			return Matrix3x3(
				s*mat.m[0][0],	s*mat.m[0][1],	s*mat.m[0][2],
				s*mat.m[1][0],	s*mat.m[1][1],	s*mat.m[1][2],
				s*mat.m[2][0],	s*mat.m[2][1],	s*mat.m[2][2]);
		}

		//! Operator for Matrix3x3 Div = Matrix3x3 / float;
		Matrix3x3		operator/(float s)				const
		{
			if (s)	s = 1.0f / s;
			return Matrix3x3(
				m[0][0]*s,	m[0][1]*s,	m[0][2]*s,
				m[1][0]*s,	m[1][1]*s,	m[1][2]*s,
				m[2][0]*s,	m[2][1]*s,	m[2][2]*s);
		}

		//! Operator for Matrix3x3 Div = float / Matrix3x3;
		friend Matrix3x3 operator/(float s, const Matrix3x3& mat)
		{
			return Matrix3x3(
				s/mat.m[0][0],	s/mat.m[0][1],	s/mat.m[0][2],
				s/mat.m[1][0],	s/mat.m[1][1],	s/mat.m[1][2],
				s/mat.m[2][0],	s/mat.m[2][1],	s/mat.m[2][2]);
		}

		//! Operator for Matrix3x3 += Matrix3x3
		Matrix3x3&		operator+=(const Matrix3x3& mat)
		{
			m[0][0] += mat.m[0][0];		m[0][1] += mat.m[0][1];		m[0][2] += mat.m[0][2];
			m[1][0] += mat.m[1][0];		m[1][1] += mat.m[1][1];		m[1][2] += mat.m[1][2];
			m[2][0] += mat.m[2][0];		m[2][1] += mat.m[2][1];		m[2][2] += mat.m[2][2];
			return	*this;
		}

		//! Operator for Matrix3x3 -= Matrix3x3
		Matrix3x3&		operator-=(const Matrix3x3& mat)
		{
			m[0][0] -= mat.m[0][0];		m[0][1] -= mat.m[0][1];		m[0][2] -= mat.m[0][2];
			m[1][0] -= mat.m[1][0];		m[1][1] -= mat.m[1][1];		m[1][2] -= mat.m[1][2];
			m[2][0] -= mat.m[2][0];		m[2][1] -= mat.m[2][1];		m[2][2] -= mat.m[2][2];
			return	*this;
		}

		//! Operator for Matrix3x3 *= Matrix3x3
		Matrix3x3&		operator*=(const Matrix3x3& mat)
		{
			D3DXVECTOR3 TempRow;

			GetRow(0, TempRow);
			m[0][0] = TempRow.x*mat.m[0][0] + TempRow.y*mat.m[1][0] + TempRow.z*mat.m[2][0];
			m[0][1] = TempRow.x*mat.m[0][1] + TempRow.y*mat.m[1][1] + TempRow.z*mat.m[2][1];
			m[0][2] = TempRow.x*mat.m[0][2] + TempRow.y*mat.m[1][2] + TempRow.z*mat.m[2][2];

			GetRow(1, TempRow);
			m[1][0] = TempRow.x*mat.m[0][0] + TempRow.y*mat.m[1][0] + TempRow.z*mat.m[2][0];
			m[1][1] = TempRow.x*mat.m[0][1] + TempRow.y*mat.m[1][1] + TempRow.z*mat.m[2][1];
			m[1][2] = TempRow.x*mat.m[0][2] + TempRow.y*mat.m[1][2] + TempRow.z*mat.m[2][2];

			GetRow(2, TempRow);
			m[2][0] = TempRow.x*mat.m[0][0] + TempRow.y*mat.m[1][0] + TempRow.z*mat.m[2][0];
			m[2][1] = TempRow.x*mat.m[0][1] + TempRow.y*mat.m[1][1] + TempRow.z*mat.m[2][1];
			m[2][2] = TempRow.x*mat.m[0][2] + TempRow.y*mat.m[1][2] + TempRow.z*mat.m[2][2];
			return	*this;
		}

		//! Operator for Matrix3x3 *= float
		Matrix3x3&		operator*=(float s)
		{
			m[0][0] *= s;	m[0][1] *= s;	m[0][2] *= s;
			m[1][0] *= s;	m[1][1] *= s;	m[1][2] *= s;
			m[2][0] *= s;	m[2][1] *= s;	m[2][2] *= s;
			return	*this;
		}

		//! Operator for Matrix3x3 /= float
		Matrix3x3&		operator/=(float s)
		{
			if (s)	s = 1.0f / s;
			m[0][0] *= s;	m[0][1] *= s;	m[0][2] *= s;
			m[1][0] *= s;	m[1][1] *= s;	m[1][2] *= s;
			m[2][0] *= s;	m[2][1] *= s;	m[2][2] *= s;
			return	*this;
		}

		// Cast operators
		//! Cast a Matrix3x3 to a Matrix4x4.
		operator D3DXMATRIX()	const;
		//! Cast a Matrix3x3 to a Quat.
		operator D3DXQUATERNION()			const;

		const D3DXVECTOR3&	operator[](int row)		const	{ return *(const D3DXVECTOR3*)&m[row][0];	}
		D3DXVECTOR3&			operator[](int row)				{ return *(D3DXVECTOR3*)&m[row][0];		}

	public:

		float			m[3][3];
	};
}
