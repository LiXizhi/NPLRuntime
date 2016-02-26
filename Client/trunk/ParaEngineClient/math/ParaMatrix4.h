#pragma once
namespace ParaEngine
{
	class QMatrix;

    /** Class encapsulating a standard 4x4 homogeneous matrix.
        @remarks
            ParaEngine uses row vectors when applying matrix multiplications,
            This means a vector is represented as a single row, 4-column
            matrix. This has the effect that the transformations implemented
            by the matrices happens left-to-right e.g. if vector V is to be
            transformed by M1 then M2 then M3, the calculation would be
            V * M1 * M2 * M3 . The order that matrices are concatenated is
            vital since matrix multiplication is not commutative, i.e. you
            can get a different result if you concatenate in the wrong order.
			But it is fine to use this class in a column-major math, the math are the same.
        @par
            ParaEngine deals with the differences between D3D and OpenGL etc.
            internally when operating through different render systems. 
        @par
            matrix is indexed first by row and then by column. 
    */
    class Matrix4
    {
	public:
        /// The matrix entries, indexed by [row][col].
        union {
			struct {
				float        _11, _12, _13, _14;
				float        _21, _22, _23, _24;
				float        _31, _32, _33, _34;
				float        _41, _42, _43, _44;
			};
            float m[4][4];
            float _m[16];
        };
    public:
        /** Default constructor.
            @note
                It does <b>NOT</b> initialize the matrix for efficiency.
        */
        inline Matrix4()
        {
        }

        inline Matrix4(
            float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33 )
        {
            m[0][0] = m00;            m[0][1] = m01;            m[0][2] = m02;            m[0][3] = m03;
            m[1][0] = m10;            m[1][1] = m11;            m[1][2] = m12;            m[1][3] = m13;
            m[2][0] = m20;            m[2][1] = m21;            m[2][2] = m22;            m[2][3] = m23;
            m[3][0] = m30;            m[3][1] = m31;            m[3][2] = m32;            m[3][3] = m33;
        }

        /** Creates a standard 4x4 transformation matrix with a zero translation part from a rotation/scaling 3x3 matrix.
         */

        inline Matrix4(const Matrix3& m3x3)
        {
          operator=(IDENTITY);
          operator=(m3x3);
        }

		inline Matrix4(const DeviceMatrix& mat)
		{
			operator=(reinterpret_cast<const Matrix4&>(mat));
		}

		/** convert from 2d affine matrix. */
		Matrix4(const QMatrix& mat2DAffine);

        /** Creates a standard 4x4 transformation matrix with a zero translation part from a rotation/scaling Quaternion.
         */
        
        inline Matrix4(const Quaternion& rot)
        {
			rot.ToRotationMatrix(*this, Vector3::ZERO);
        }
        
		/** make this identity*/
		inline void identity() {
			*this = Matrix4::IDENTITY;
		};

        inline float* operator [] ( size_t iRow )
        {
            assert( iRow < 4 );
            return m[iRow];
        }

        inline const float *const operator [] ( size_t iRow ) const
        {
            assert( iRow < 4 );
            return m[iRow];
        }

        /** Matrix4 concatenation using '*'.
        */
        inline Matrix4 operator * ( const Matrix4 &m2 ) const
        {
			Matrix4 r;
			ParaMatrixMultiply(&r, this, &m2);
			return r;
        }

		/** we will assume both matrix are row-major affine matrix. column 3 is (0,0,0,1) */
		Matrix4 Multiply4x3(const Matrix4 &m2) const;

		inline void operator *= (const Matrix4 &m2)
		{
			ParaMatrixMultiply(this, this, &m2);
		}

        /** Vector transformation using '*'.
            @remarks
                Transforms the given 3-D vector by the matrix, projecting the 
                result back into <i>w</i> = 1.
            @note
                This means that the initial <i>w</i> is considered to be 1.0,
                and then all the tree elements of the resulting 3-D vector are
                divided by the resulting <i>w</i>.
        */
        inline Vector3 operator * ( const Vector3 &v ) const
        {
            Vector3 r;

            float fInvW = 1.0f / ( m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] );

            r.x = ( m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] ) * fInvW;
            r.y = ( m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] ) * fInvW;
            r.z = ( m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] ) * fInvW;

            return r;
        }
        inline Vector4 operator * (const Vector4& v) const
        {
            return Vector4(
                m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w, 
                m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
                m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
                m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
                );
        }
        inline Plane operator * (const Plane& p) const
        {
            Plane ret;
			Matrix4 invTrans = inverse().transpose();
			Vector4 v4( p.normal.x, p.normal.y, p.normal.z, p.d );
			v4 = invTrans * v4;
			ret.normal.x = v4.x; 
			ret.normal.y = v4.y; 
			ret.normal.z = v4.z;
			ret.d = v4.w / ret.normal.normalise();

            return ret;
        }


        /** Matrix4 addition.
        */
        inline Matrix4 operator + ( const Matrix4 &m2 ) const
        {
            Matrix4 r;

            r.m[0][0] = m[0][0] + m2.m[0][0];
            r.m[0][1] = m[0][1] + m2.m[0][1];
            r.m[0][2] = m[0][2] + m2.m[0][2];
            r.m[0][3] = m[0][3] + m2.m[0][3];

            r.m[1][0] = m[1][0] + m2.m[1][0];
            r.m[1][1] = m[1][1] + m2.m[1][1];
            r.m[1][2] = m[1][2] + m2.m[1][2];
            r.m[1][3] = m[1][3] + m2.m[1][3];

            r.m[2][0] = m[2][0] + m2.m[2][0];
            r.m[2][1] = m[2][1] + m2.m[2][1];
            r.m[2][2] = m[2][2] + m2.m[2][2];
            r.m[2][3] = m[2][3] + m2.m[2][3];

            r.m[3][0] = m[3][0] + m2.m[3][0];
            r.m[3][1] = m[3][1] + m2.m[3][1];
            r.m[3][2] = m[3][2] + m2.m[3][2];
            r.m[3][3] = m[3][3] + m2.m[3][3];

            return r;
        }
		inline Matrix4& operator += (const Matrix4 &m2)
		{
			m[0][0] += m2.m[0][0];			m[0][1] += m2.m[0][1];			m[0][2] += m2.m[0][2];			m[0][3] += m2.m[0][3];
			m[1][0] += m2.m[1][0];			m[1][1] += m2.m[1][1];			m[1][2] += m2.m[1][2];			m[1][3] += m2.m[1][3];
			m[2][0] += m2.m[2][0];			m[2][1] += m2.m[2][1];			m[2][2] += m2.m[2][2];			m[2][3] += m2.m[2][3];
			m[3][0] += m2.m[3][0];			m[3][1] += m2.m[3][1];			m[3][2] += m2.m[3][2];			m[3][3] += m2.m[3][3];
			return *this;
		}
		
        /** Matrix4 subtraction.
        */
        inline Matrix4 operator - ( const Matrix4 &m2 ) const
        {
            Matrix4 r;
            r.m[0][0] = m[0][0] - m2.m[0][0];
            r.m[0][1] = m[0][1] - m2.m[0][1];
            r.m[0][2] = m[0][2] - m2.m[0][2];
            r.m[0][3] = m[0][3] - m2.m[0][3];

            r.m[1][0] = m[1][0] - m2.m[1][0];
            r.m[1][1] = m[1][1] - m2.m[1][1];
            r.m[1][2] = m[1][2] - m2.m[1][2];
            r.m[1][3] = m[1][3] - m2.m[1][3];

            r.m[2][0] = m[2][0] - m2.m[2][0];
            r.m[2][1] = m[2][1] - m2.m[2][1];
            r.m[2][2] = m[2][2] - m2.m[2][2];
            r.m[2][3] = m[2][3] - m2.m[2][3];

            r.m[3][0] = m[3][0] - m2.m[3][0];
            r.m[3][1] = m[3][1] - m2.m[3][1];
            r.m[3][2] = m[3][2] - m2.m[3][2];
            r.m[3][3] = m[3][3] - m2.m[3][3];

            return r;
        }
		inline Matrix4& operator -= (const Matrix4 &m2)
		{
			m[0][0] -= m2.m[0][0];			m[0][1] -= m2.m[0][1];			m[0][2] -= m2.m[0][2];			m[0][3] -= m2.m[0][3];
			m[1][0] -= m2.m[1][0];			m[1][1] -= m2.m[1][1];			m[1][2] -= m2.m[1][2];			m[1][3] -= m2.m[1][3];
			m[2][0] -= m2.m[2][0];			m[2][1] -= m2.m[2][1];			m[2][2] -= m2.m[2][2];			m[2][3] -= m2.m[2][3];
			m[3][0] -= m2.m[3][0];			m[3][1] -= m2.m[3][1];			m[3][2] -= m2.m[3][2];			m[3][3] -= m2.m[3][3];
			return *this;
		}

        /** Tests 2 matrices for equality.
        */
        inline bool operator == ( const Matrix4& m2 ) const
        {
            if( 
                m[0][0] != m2.m[0][0] || m[0][1] != m2.m[0][1] || m[0][2] != m2.m[0][2] || m[0][3] != m2.m[0][3] ||
                m[1][0] != m2.m[1][0] || m[1][1] != m2.m[1][1] || m[1][2] != m2.m[1][2] || m[1][3] != m2.m[1][3] ||
                m[2][0] != m2.m[2][0] || m[2][1] != m2.m[2][1] || m[2][2] != m2.m[2][2] || m[2][3] != m2.m[2][3] ||
                m[3][0] != m2.m[3][0] || m[3][1] != m2.m[3][1] || m[3][2] != m2.m[3][2] || m[3][3] != m2.m[3][3] )
                return false;
            return true;
        }

        /** Tests 2 matrices for inequality.
        */
        inline bool operator != ( const Matrix4& m2 ) const
        {
            if( 
                m[0][0] != m2.m[0][0] || m[0][1] != m2.m[0][1] || m[0][2] != m2.m[0][2] || m[0][3] != m2.m[0][3] ||
                m[1][0] != m2.m[1][0] || m[1][1] != m2.m[1][1] || m[1][2] != m2.m[1][2] || m[1][3] != m2.m[1][3] ||
                m[2][0] != m2.m[2][0] || m[2][1] != m2.m[2][1] || m[2][2] != m2.m[2][2] || m[2][3] != m2.m[2][3] ||
                m[3][0] != m2.m[3][0] || m[3][1] != m2.m[3][1] || m[3][2] != m2.m[3][2] || m[3][3] != m2.m[3][3] )
                return true;
            return false;
        }

        /** Assignment from 3x3 matrix.
        */
        inline void operator = ( const Matrix3& mat3 )
        {
            m[0][0] = mat3.m[0][0]; m[0][1] = mat3.m[0][1]; m[0][2] = mat3.m[0][2];
            m[1][0] = mat3.m[1][0]; m[1][1] = mat3.m[1][1]; m[1][2] = mat3.m[1][2];
            m[2][0] = mat3.m[2][0]; m[2][1] = mat3.m[2][1]; m[2][2] = mat3.m[2][2];
        }

        inline Matrix4 transpose(void) const
        {
            return Matrix4(m[0][0], m[1][0], m[2][0], m[3][0],
                           m[0][1], m[1][1], m[2][1], m[3][1],
                           m[0][2], m[1][2], m[2][2], m[3][2],
                           m[0][3], m[1][3], m[2][3], m[3][3]);
        }

        /*
        -----------------------------------------------------------------------
        Translation Transformation
        -----------------------------------------------------------------------
        */
		inline void offsetTrans(const Vector3& v)
		{
			m[3][0] += v.x; 			m[3][1] += v.y;			m[3][2] += v.z;
		}

        /** Sets the translation transformation part of the matrix.
        */
        inline void setTrans( const Vector3& v )
        {
            m[3][0] = v.x;
            m[3][1] = v.y;
            m[3][2] = v.z;
        }

        /** Extracts the translation transformation part of the matrix.
         */
        inline Vector3 getTrans() const
        {
          return Vector3(m[3][0], m[3][1], m[3][2]);
        }
        

        /** Builds a translation matrix
        */
        inline void makeTrans( const Vector3& v )
        {
			m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0; m[0][3] = 0.0;
			m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0; m[1][3] = 0.0;
			m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0; m[2][3] = 0.0;
			m[3][0] = v.x; m[3][1] = v.y; m[3][2] = v.z; m[3][3] = 1.0;
        }

        inline void makeTrans( float tx, float ty, float tz )
        {
			m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0; m[0][3] = 0.0;
            m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0; m[1][3] = 0.0;
			m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0; m[2][3] = 0.0;
			m[3][0] = tx;  m[3][1] = ty;  m[3][2] = tz;  m[3][3] = 1.0;
        }

        
        /*
        -----------------------------------------------------------------------
        Scale Transformation
        -----------------------------------------------------------------------
        */
        /** Sets the scale part of the matrix.
        */
        inline void setScale( const Vector3& v )
        {
            m[0][0] = v.x;
            m[1][1] = v.y;
            m[2][2] = v.z;
        }

        /** Gets a scale matrix - variation for not using a vector.
        */
        inline void makeScale( float s_x, float s_y, float s_z )
        {
			identity();
            m[0][0] = s_x; m[1][1] = s_y; m[2][2] = s_z; 
        }

		inline void makeScale(const Vector3& v)
		{
			identity();
			m[0][0] = v.x; m[1][1] = v.y; m[2][2] = v.z;
		}

        /** Extracts the rotation / scaling part of the Matrix4 as a 3x3 matrix. 
        @param m3x3 Destination Matrix3
        */
        inline void extract3x3Matrix(Matrix3& m3x3) const
        {
            m3x3.m[0][0] = m[0][0];            m3x3.m[0][1] = m[0][1];            m3x3.m[0][2] = m[0][2];
            m3x3.m[1][0] = m[1][0];            m3x3.m[1][1] = m[1][1];            m3x3.m[1][2] = m[1][2];
            m3x3.m[2][0] = m[2][0];            m3x3.m[2][1] = m[2][1];            m3x3.m[2][2] = m[2][2];
        }

		/** Determines if this matrix involves a scaling. */
		bool hasScale() const;

		/** Determines if this matrix involves a negative scaling. */
		inline bool hasNegativeScale() const
		{
			return determinant() < 0;
		}

		/** Extracts the rotation / scaling part as a quaternion from the Matrix4.
         */
        inline Quaternion extractQuaternion() const
        {
          return Quaternion(*this);
        }

        static const Matrix4 ZERO;
        static const Matrix4 IDENTITY;
        /** Useful little matrix which takes 2D clipspace {-1, 1} to {0,1}
            and inverts the Y. */
        static const Matrix4 CLIPSPACE2DTOIMAGESPACE;

        inline Matrix4 operator*(float scalar) const
        {
            return Matrix4(
                scalar*m[0][0], scalar*m[0][1], scalar*m[0][2], scalar*m[0][3],
                scalar*m[1][0], scalar*m[1][1], scalar*m[1][2], scalar*m[1][3],
                scalar*m[2][0], scalar*m[2][1], scalar*m[2][2], scalar*m[2][3],
                scalar*m[3][0], scalar*m[3][1], scalar*m[3][2], scalar*m[3][3]);
        }
		
        /** Function for writing to a stream.
        */
        friend std::ostream& operator << ( std::ostream& o, const Matrix4& m )
        {
            o << "Matrix4(";
			for (size_t i = 0; i < 4; ++i)
            {
                o << " row" << (unsigned)i << "{";
                for(size_t j = 0; j < 4; ++j)
                {
                    o << m[i][j] << " ";
                }
                o << "}";
            }
            o << ")";
            return o;
        }
		
		Matrix4 adjoint() const;
		float determinant() const;
		Matrix4 inverse() const;
		inline void invert() {
			*this = inverse();
		};

		/**
		*	Inverts a PR matrix. (which only contains a rotation and a translation)
		*	This is faster and less subject to FPU errors than the generic inversion code.
		*
		*	@param		dest	[out] destination matrix
		*/
		Matrix4 InvertPRMatrix() const;

        /** Building a Matrix4 from orientation / scale / position.
        @remarks
            Transform is performed in the order scale, rotate, translation, i.e. translation is independent
            of orientation axes, scale does not affect size of translation, rotation and scaling are always
            centered on the origin.
        */
        void makeTransform(const Vector3& position, const Vector3& scale, const Quaternion& orientation);

		/** make rotation matrix*/
		void makeRot(const Quaternion& orientation, const Vector3& origin);

        /** Building an inverse Matrix4 from orientation / scale / position.
        @remarks
            As makeTransform except it build the inverse given the same data as makeTransform, so
            performing -translation, -rotate, 1/scale in that order.
        */
        void makeInverseTransform(const Vector3& position, const Vector3& scale, const Quaternion& orientation);

        /** Check whether or not the matrix is affine row-major matrix.
            @remarks
                An affine matrix is a 4x4 matrix with column 3 equal to (0, 0, 0, 1),
                e.g. no projective coefficients.
        */
        inline bool isAffine(void) const
        {
            return m[0][3] == 0 && m[1][3] == 0 && m[2][3] == 0 && m[3][3] == 1;
        }
		inline bool isAffineColumnMajor(void) const
		{
			return m[3][0] == 0 && m[3][1] == 0 && m[3][2] == 0 && m[3][3] == 1;
		}


        /** Column vector: 3-D Vector transformation specially for affine matrix.
            @remarks
                Transforms the given 3-D vector by the matrix, projecting the 
                result back into <i>w</i> = 1.
            @note
                The matrix must be an affine matrix. @see Matrix4::isAffine.
        */
        inline Vector3 transformAffine(const Vector3& v) const
        {
			// assert(isAffineColumnMajor());

            return Vector3(
                    m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3], 
                    m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3],
                    m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]);
        }

        /** Column vector: 4-D Vector transformation specially for affine matrix.
            @note
                The matrix must be an affine matrix. @see Matrix4::isAffine.
        */
        inline Vector4 transformAffine(const Vector4& v) const
        {
			// assert(isAffineColumnMajor());

            return Vector4(
                m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w, 
                m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
                m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
                v.w);
        }


		// NOTE: There is some compiler optimization issues with WIN64 that cause FORCEINLINE to cause a crash
		// Remove any scaling from this matrix (ie magnitude of each row is 1)
		void RemoveScaling(float Tolerance=SMALL_NUMBER);

		// Returns matrix without scale information
		Matrix4 GetMatrixWithoutScale(float Tolerance=SMALL_NUMBER) const;

		/** Remove any scaling from this matrix (ie magnitude of each row is 1) and return the 3D scale vector that was initially present. */
		Vector3 ExtractScaling(float Tolerance=SMALL_NUMBER);

		/** return a 3D scale vector calculated from this matrix (where each component is the magnitude of a row vector). */
		Vector3 GetScaleVector(float Tolerance=SMALL_NUMBER) const;
		/**
		* @param axis: 0,1,2 for x,y,z
		*/
		float GetScaleByAxis(int axis, float Tolerance = SMALL_NUMBER) const;
		// Remove any translation from this matrix
		Matrix4 RemoveTranslation() const;

		inline DeviceMatrix_ptr GetPointer() { return (DeviceMatrix_ptr)this; }
		inline const DeviceMatrix_ptr GetConstPointer() const { return (const DeviceMatrix_ptr)this; }
    };
}
