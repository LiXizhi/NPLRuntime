#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

namespace ParaEngine
{
	/** Engine capability version queried by NPL scripts via ParaEngine.GetAttributeObject():GetField("CapeSystemVersion", 0) */
	static constexpr int CAPE_SYSTEM_VERSION = 1;

	enum class CapeMode : uint8_t
	{
		None = 0,
		ProceduralMesh = 1,
		GeosetTakeover = 2,
	};

	enum class CapeCollarStyle : uint8_t
	{
		WrapPoncho = 0,  // Sky-style horseshoe wrap over shoulders and upper chest
		ClassicBack = 1, // Traditional shoulder-to-shoulder back cloak
		Scarf = 2,       // Narrow streamer / ribbon scarf
	};

	enum class CapeHemStyle : uint8_t
	{
		Rounded = 0,     // Smooth fan arc
		Scalloped = 1,   // Petal / wave lobes
		StarPointed = 2, // Alternating star tips and valleys
		Swallowtail = 3, // Central split V/U tail
		Pointed = 4,     // Single central pointed tip
		Asymmetric = 5,  // Diagonal slanted hem
	};

	struct CapeVec2
	{
		float x = 0.0f;
		float y = 0.0f;
	};

	struct CapeVec3
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;

		CapeVec3() = default;
		constexpr CapeVec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

		CapeVec3 operator+(const CapeVec3& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
		CapeVec3 operator-(const CapeVec3& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
		CapeVec3 operator*(float s) const { return { x * s, y * s, z * s }; }
		CapeVec3& operator+=(const CapeVec3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
		CapeVec3& operator-=(const CapeVec3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
		CapeVec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }

		float Dot(const CapeVec3& rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }
		CapeVec3 Cross(const CapeVec3& rhs) const
		{
			return {
				y * rhs.z - z * rhs.y,
				z * rhs.x - x * rhs.z,
				x * rhs.y - y * rhs.x
			};
		}
		float LengthSq() const { return x * x + y * y + z * z; }
		float Length() const { return std::sqrt(LengthSq()); }
		CapeVec3 Normalized(const CapeVec3& fallback = { 0.0f, 1.0f, 0.0f }) const
		{
			const float lenSq = LengthSq();
			if (lenSq > 1e-10f)
			{
				const float inv = 1.0f / std::sqrt(lenSq);
				return { x * inv, y * inv, z * inv };
			}
			return fallback;
		}
	};

	inline CapeVec3 CapeLerp(const CapeVec3& a, const CapeVec3& b, float t)
	{
		return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
	}

	struct CapeCapsule
	{
		CapeVec3 a;
		CapeVec3 b;
		float radius = 0.1f;
	};

	/**
	 * Canonical standing character body collider in character-local space (+X forward, +Y up, -Z right).
	 * Assumes the character's torso, shoulders/arms, hips/tunic, and legs occupy a solid 8-slice
	 * envelope from baseY (feet) up to neckY, preventing cape cloth from passing through the body
	 * when standing idle, running, turning, or walking backward.
	 */
	struct CapeBodyCollider
	{
		static constexpr int NUM_SLICES = 8;
		bool enabled = true;
		float baseY = 0.0f;
		float neckY = 1.20f;
		float centerX = 0.0f;
		float centerZ = 0.0f;
		float headBackRadius = 0.16f;
		float backRadius[NUM_SLICES] = {};
		float frontRadius[NUM_SLICES] = {};
		float sideRadius[NUM_SLICES] = {};

		void InitCanonical(
			float inBaseY,
			float inNeckY,
			float shoulderHalfSpan,
			float modelScale = 1.0f,
			float inCenterX = 0.0f,
			float inCenterZ = 0.0f,
			float inBackSurfaceDepth = 0.0f)
		{
			enabled = true;
			const float sc = (modelScale > 0.05f) ? modelScale : 1.0f;
			baseY = inBaseY;
			neckY = (std::max)(inBaseY + 0.32f * sc, inNeckY);
			centerX = inCenterX;
			centerZ = inCenterZ;

			const float bodyH = neckY - baseY;
			const float shHalf = (std::max)(0.125f * sc, (std::min)(0.24f * sc, (std::max)(shoulderHalfSpan, bodyH * 0.215f)));
			const float torsoDepth = (inBackSurfaceDepth > 0.03f)
				? (std::max)(0.075f * sc, (std::min)(bodyH * 0.26f, inBackSurfaceDepth))
				: (std::max)(0.085f * sc, (std::min)(0.165f * sc, bodyH * 0.185f));
			headBackRadius = (std::max)(0.14f * sc, torsoDepth * 1.12f);

			// 8 canonical standing-body slices from s=0 (collar/neck) down to s=1 (ankles/boots)
			static constexpr float kBackMul[NUM_SLICES]  = { 0.92f, 1.03f, 1.10f, 1.18f, 1.30f, 1.16f, 1.02f, 0.92f };
			static constexpr float kFrontMul[NUM_SLICES] = { 0.84f, 0.96f, 1.04f, 1.04f, 1.06f, 0.96f, 0.88f, 0.90f };
			static constexpr float kSideMul[NUM_SLICES]  = { 0.80f, 1.06f, 1.06f, 1.04f, 1.08f, 0.94f, 0.84f, 0.78f };

			for (int i = 0; i < NUM_SLICES; ++i)
			{
				backRadius[i]  = torsoDepth * kBackMul[i];
				frontRadius[i] = torsoDepth * kFrontMul[i];
				sideRadius[i]  = shHalf * kSideMul[i];
			}
		}

		void SampleSlice(float sNorm, float& outBackR, float& outFrontR, float& outSideR) const
		{
			const float clamped = (std::max)(0.0f, (std::min)(1.0f, sNorm));
			const float fIdx = clamped * static_cast<float>(NUM_SLICES - 1);
			const int i0 = (std::min)(NUM_SLICES - 2, static_cast<int>(fIdx));
			const int i1 = i0 + 1;
			const float t = fIdx - static_cast<float>(i0);
			outBackR  = backRadius[i0]  + (backRadius[i1]  - backRadius[i0])  * t;
			outFrontR = frontRadius[i0] + (frontRadius[i1] - frontRadius[i0]) * t;
			outSideR  = sideRadius[i0]  + (sideRadius[i1]  - sideRadius[i0])  * t;
		}

		bool CollideParticle(
			CapeVec3& p,
			CapeVec3* ioPrevPos,
			float uNorm,
			float vNorm,
			const CapeVec3& forwardDir,
			const CapeVec3& upDir,
			const CapeVec3& rightDir,
			float horizSpeed,
			bool isFlying) const
		{
			if (!enabled)
				return false;

			const float bodyH = (std::max)(0.25f, neckY - baseY);
			const CapeVec3 spineOrigin = forwardDir * centerX + rightDir * centerZ;
			const CapeVec3 rel = p - spineOrigin;
			float dx = rel.Dot(forwardDir); // >0 front, <0 back
			const float y = rel.Dot(upDir);
			float dz = rel.Dot(rightDir);   // left/right
			bool modified = false;

			// 1. Ground plane clearance when standing/running so long capes trail behind heels instead of sinking underground
			const float floorY = baseY + 0.02f;
			if (!isFlying && y < floorY)
			{
				const float liftUp = floorY - y;
				p += upDir * liftUp - forwardDir * (liftUp * 0.35f);
				dx -= liftUp * 0.35f;
				if (ioPrevPos)
					*ioPrevPos += upDir * (liftUp * 0.5f);
				modified = true;
			}

			// 2. Back-of-head / hair sphere clearance above neckY when jumping or billowing high
			if (y > neckY + 0.02f && y < neckY + bodyH * 0.90f)
			{
				const float headCenterY = neckY + headBackRadius * 0.85f;
				const float dy = y - headCenterY;
				const float rHead = headBackRadius;
				const float dSq = dx * dx + dy * dy + dz * dz;
				if (dSq < rHead * rHead)
				{
					float pushX = dx;
					float pushZ = dz;
					if (pushX > -rHead * 0.35f && std::fabs(pushZ) < rHead * 0.80f)
					{
						pushX = -rHead * 0.85f;
					}
					const float norm = std::sqrt(pushX * pushX + dy * dy + pushZ * pushZ);
					if (norm > 1e-5f)
					{
						const float k = rHead / norm;
						const CapeVec3 newPos = spineOrigin + forwardDir * (pushX * k) + upDir * (headCenterY + dy * k) + rightDir * (pushZ * k);
						const CapeVec3 corr = newPos - p;
						p = newPos;
						if (ioPrevPos)
							*ioPrevPos += corr * 0.35f;
						return true;
					}
				}
				return modified;
			}

			if (y < baseY - 0.15f || y > neckY + 0.04f)
				return modified;

			// 3. 8-slice torso / shoulder / hip / leg envelope with anti-tunneling back-hemisphere guard
			const float s = (std::max)(0.0f, (std::min)(1.0f, (neckY - y) / bodyH));
			float backR = 0.15f, frontR = 0.14f, sideR = 0.18f;
			SampleSlice(s, backR, frontR, sideR);

			// Expand mid- and lower-body envelope during walking/running to cover swinging elbows, tunic hems, and stride
			if (s > 0.20f && horizSpeed > 0.15f && !isFlying)
			{
				const float runFactor = (std::min)(1.0f, horizSpeed / 4.2f);
				const float swingT = (s - 0.20f) / 0.80f;
				backR += runFactor * (0.045f + 0.22f * swingT) * bodyH;
				sideR += runFactor * (0.030f + 0.045f * swingT) * bodyH;
			}

			const float absZ = std::fabs(dz);
			const bool isOuterWrapEdge = (vNorm < 0.16f) && (uNorm < 0.16f || uNorm > 0.84f);

			// Coronal anti-tunneling barrier: interior cape cloth can NEVER cross through the torso/arms into the front (+X)
			if (!isOuterWrapEdge && absZ < sideR * 1.32f)
			{
				const float sideBlend = (std::min)(1.0f, absZ / (sideR * 1.15f)); // 0 at spine -> 1 past outer arm edge
				const float maxAllowedDx = -backR * (0.52f - 0.34f * sideBlend) - 0.015f;
				if (dx > maxAllowedDx)
				{
					dx = maxAllowedDx;
					modified = true;
				}
			}

			const float rx = (dx <= 0.0f) ? backR : frontR;
			const float rz = (std::max)(0.05f, sideR);
			float nx = dx / (std::max)(0.05f, rx);
			float nz = dz / rz;
			const float ellipSq = nx * nx + nz * nz;
			// Rounded-box (superellipse) corner expansion so boxy coats/tunics & side-by-side legs don't poke out at 45 degrees
			const float corner2 = (ellipSq > 1e-6f) ? ((nx * nx * nz * nz) / (ellipSq * ellipSq)) : 0.0f;
			const float targetR = 1.0f + 0.72f * corner2;

			if (ellipSq < targetR * targetR)
			{
				// Bias near-center penetrations toward the back hemisphere so cloth parts cleanly around back/sides
				if (!isOuterWrapEdge && nx > -0.25f && std::fabs(nz) < 0.85f)
				{
					nx = -0.75f;
					nz += (uNorm < 0.5f) ? -0.25f : 0.25f;
				}
				const float len = std::sqrt(nx * nx + nz * nz);
				if (len > 1e-5f)
				{
					const float ux = nx / len;
					const float uz = nz / len;
					const float outTargetR = 1.0f + 0.72f * (ux * ux * uz * uz);
					nx = ux * outTargetR;
					nz = uz * outTargetR;
				}
				else
				{
					nx = -1.0f;
					nz = 0.0f;
				}
				const float outRx = (nx <= 0.0f) ? backR : frontR;
				dx = nx * outRx;
				dz = nz * rz;
				modified = true;
			}

			if (modified)
			{
				const CapeVec3 newPos = spineOrigin + forwardDir * dx + upDir * y + rightDir * dz;
				const CapeVec3 corr = newPos - p;
				p = newPos;
				if (ioPrevPos)
				{
					// Dampen normal bounce so cape glides smoothly against the character's back
					*ioPrevPos += corr * 0.40f;
				}
			}
			return modified;
		}
	};

	struct CapeStyleParams
	{
		CapeCollarStyle collarStyle = CapeCollarStyle::WrapPoncho;
		CapeHemStyle hemStyle = CapeHemStyle::Rounded;
		int gridCols = 9;
		int gridRows = 7;
		float length = 0.88f;
		float topWidth = 0.36f;
		float bottomWidth = 0.92f;
		float wrapAngleDeg = 220.0f;
		float shoulderDrop = 0.05f;
		float backOffset = 0.06f;
		float bellCurvature = 0.10f;
		int hemCount = 5;
		float hemAmplitude = 0.08f;
		float splitDepth = 0.35f;
		CapeVec3 attachOffset = { 0.0f, 0.0f, 0.0f };
	};

	struct CapeSimTuning
	{
		float gravity = 9.2f;
		float damping = 0.04f;
		float bendStiffness = 0.45f;
		float stretchLimit = 1.08f;
		float windResponse = 1.0f;
		float liftFactor = 0.65f;
	};

	struct CapeMotionInput
	{
		/** Character displacement in true world space over dt, rotated into character local space */
		CapeVec3 localVelocity = { 0.0f, 0.0f, 0.0f };
		/** Yaw angular velocity in rad/s (positive = turning around +Y) */
		float yawAngularVel = 0.0f;
		/** External wind vector in character local space */
		CapeVec3 localWind = { 0.0f, 0.0f, 0.0f };
		/** Character local forward/up/right basis (ParaEngine left-handed: +X forward, +Y up, -Z right) */
		CapeVec3 forwardDir = { 1.0f, 0.0f, 0.0f };
		CapeVec3 upDir = { 0.0f, 1.0f, 0.0f };
		CapeVec3 rightDir = { 0.0f, 0.0f, -1.0f };
		bool isFlying = false;
		bool teleported = false;
		float timeSeconds = 0.0f;
	};

	struct CapeParticle
	{
		CapeVec3 pos;
		CapeVec3 prevPos;
		CapeVec3 restLocalPos; // Natural bell-drape rest pose in character local space
		CapeVec2 uv;
		float invMass = 1.0f;  // 0 = pinned to collar/shoulders
		float uNorm = 0.0f;    // [0..1] across width
		float vNorm = 0.0f;    // [0..1] from collar to hem
		int anchorIndex = 0;   // Top-row column index for tether constraint
		float tetherMaxDist = 0.0f;
		// Shoulder/neck blend weights: [0]=Neck/Spine, [1]=LeftShoulder, [2]=RightShoulder
		float anchorWeights[3] = { 1.0f, 0.0f, 0.0f };
		CapeVec3 anchorLocalOffsets[3];
	};

	struct CapeDistanceConstraint
	{
		uint16_t i0 = 0;
		uint16_t i1 = 0;
		float restLength = 0.0f;
		float stiffness = 1.0f; // 1.0 for structural, <1 for shear/bending
	};

	struct CapeSurfaceVertex
	{
		CapeVec3 pos;
		CapeVec3 normal;
		CapeVec2 uv;
	};

	/** Evaluate sculpted hem length multiplier at normalized width u in [0, 1] */
	inline float EvaluateCapeHemFactor(const CapeStyleParams& style, float u)
	{
		static constexpr float kPi = 3.14159265358979323846f;
		const float centered = (u - 0.5f) * 2.0f; // [-1, 1]
		const float absC = std::fabs(centered);
		const int lobes = (std::max)(2, (std::min)(16, style.hemCount));
		const float amp = (std::max)(0.0f, (std::min)(0.45f, style.hemAmplitude));
		float factor = 1.0f;

		switch (style.hemStyle)
		{
		case CapeHemStyle::Rounded:
			// Longer in the center, curving smoothly up toward the sides like Sky's classic cape
			factor = 1.0f - 0.16f * centered * centered;
			break;
		case CapeHemStyle::Scalloped:
		{
			const float baseArc = 1.0f - 0.12f * centered * centered;
			const float wave = std::fabs(std::sin(u * static_cast<float>(lobes) * kPi));
			factor = baseArc - amp * (1.0f - wave);
			break;
		}
		case CapeHemStyle::StarPointed:
		{
			const float baseArc = 1.0f - 0.10f * centered * centered;
			const float saw = std::fabs(std::fmod(u * static_cast<float>(lobes), 1.0f) * 2.0f - 1.0f);
			// Sharp tips at saw==1, curved valleys at saw==0
			factor = baseArc + amp * (saw * saw * 1.4f - 0.6f);
			break;
		}
		case CapeHemStyle::Swallowtail:
		{
			const float split = (std::max)(0.05f, (std::min)(0.75f, style.splitDepth));
			// Twin tails peaking around |centered| ~= 0.55, deep V notch at center (centered == 0)
			const float notch = (1.0f - (std::min)(1.0f, absC / 0.55f));
			const float outerTaper = (absC > 0.55f) ? ((absC - 0.55f) / 0.45f) : 0.0f;
			factor = 1.05f - split * notch * notch - 0.22f * outerTaper * outerTaper;
			break;
		}
		case CapeHemStyle::Pointed:
		{
			factor = 1.08f - (0.32f + amp) * std::pow(absC, 1.35f);
			break;
		}
		case CapeHemStyle::Asymmetric:
		{
			const float slant = 0.82f + 0.34f * u;
			const float gentleWave = amp * 0.5f * std::sin(u * kPi * 2.0f);
			factor = slant + gentleWave - 0.08f * centered * centered;
			break;
		}
		}
		return (std::max)(0.25f, factor);
	}

	struct CapeClothMesh
	{
		int cols = 0;
		int rows = 0;
		std::vector<CapeParticle> particles;
		std::vector<CapeDistanceConstraint> constraints;
		std::vector<uint16_t> triIndices; // Single-layer CCW indices (outer surface)
		CapeBodyCollider bodyCollider;
		float simAccumulator = 0.0f;

		void BuildProcedural(
			const CapeStyleParams& rawStyle,
			const CapeVec3& neckAnchor,
			const CapeVec3& leftShoulderAnchor,
			const CapeVec3& rightShoulderAnchor,
			const CapeVec3& forwardDir,
			const CapeVec3& upDir,
			const CapeVec3& rightDir,
			float modelScale = 1.0f,
			const CapeBodyCollider* customBody = nullptr)
		{
			static constexpr float kPi = 3.14159265358979323846f;
			cols = (std::max)(4, (std::min)(15, rawStyle.gridCols));
			rows = (std::max)(4, (std::min)(12, rawStyle.gridRows));
			if (rawStyle.collarStyle == CapeCollarStyle::Scarf)
			{
				cols = (std::max)(4, (std::min)(7, cols));
			}

			const CapeVec3 backDir = forwardDir * -1.0f;
			const float scale = (modelScale > 0.05f) ? modelScale : 1.0f;
			if (customBody)
			{
				bodyCollider = *customBody;
			}
			else if (bodyCollider.backRadius[0] <= 1e-4f)
			{
				const float shHalf = 0.5f * std::fabs((rightShoulderAnchor - leftShoulderAnchor).Dot(rightDir));
				bodyCollider.InitCanonical(0.0f, neckAnchor.Dot(upDir), shHalf, scale, 0.0f, 0.0f);
			}

			const float topW = (rawStyle.collarStyle == CapeCollarStyle::Scarf ? rawStyle.topWidth * 0.42f : rawStyle.topWidth) * scale;
			const float botW = (rawStyle.collarStyle == CapeCollarStyle::Scarf ? rawStyle.bottomWidth * 0.32f : rawStyle.bottomWidth) * scale;
			const float capeLen = (std::max)(0.15f, rawStyle.length * scale);
			const float wrapRad = (std::max)(60.0f, (std::min)(280.0f, rawStyle.wrapAngleDeg)) * (kPi / 180.0f);
			const float backOff = rawStyle.backOffset * scale;
			const float bell = rawStyle.bellCurvature * scale;
			const float drop = rawStyle.shoulderDrop * scale;
			const CapeVec3 userOffset = rawStyle.attachOffset * scale;

			// Anchor collar relative to canonical spine X/Z at neck height so neck bone offsets never bury collar inside chest
			const CapeVec3 spineCollarOrigin = forwardDir * bodyCollider.centerX
				+ upDir * neckAnchor.Dot(upDir)
				+ rightDir * bodyCollider.centerZ
				+ userOffset;

			const float collarMinBackR = bodyCollider.enabled ? (bodyCollider.backRadius[0] * 1.03f) : (0.10f * scale);
			const float collarMinFrontR = bodyCollider.enabled ? (bodyCollider.frontRadius[0] * 1.04f) : (0.09f * scale);
			const float collarMinSideR = bodyCollider.enabled ? (bodyCollider.sideRadius[0] * 1.02f) : (0.13f * scale);

			// ClassicBack / Scarf clasp slightly lower on the upper shoulder blades (aligned with ATT_ID_BACK_ADDON)
			const float classicDropY = (rawStyle.collarStyle == CapeCollarStyle::WrapPoncho) ? 0.0f : (0.024f * scale);
			const CapeVec3 effectiveCollarOrigin = spineCollarOrigin - upDir * classicDropY;

			particles.clear();
			particles.resize(static_cast<size_t>(cols * rows));
			constraints.clear();
			triIndices.clear();

			for (int r = 0; r < rows; ++r)
			{
				const float v = static_cast<float>(r) / static_cast<float>(rows - 1);
				for (int c = 0; c < cols; ++c)
				{
					const float u = static_cast<float>(c) / static_cast<float>(cols - 1);
					const float s = (u - 0.5f) * 2.0f; // [-1..1] left to right
					const float hemMul = EvaluateCapeHemFactor(rawStyle, u);
					const float rowLen = capeLen * hemMul * v;

					CapeVec3 pLocal;
					if (rawStyle.collarStyle == CapeCollarStyle::WrapPoncho)
					{
						// Snug oval horseshoe collar hugging the neck/collarbone & upper back (wider side-to-side than front-to-back)
						const float theta = s * (wrapRad * 0.5f);
						const float effTheta = theta * (1.0f - 0.32f * v);
						const float cosT = std::cos(effTheta);
						const float sinT = std::sin(effTheta);

						const float collarRx = (cosT >= 0.0f)
							? (collarMinBackR + 0.006f * scale)
							: (collarMinFrontR + 0.008f * scale);
						const float collarRz = (std::max)(topW * 0.48f, collarMinSideR * 1.04f);

						const float radiusX = collarRx * (1.0f - v) + (botW * 0.36f + bell * std::sin(v * kPi * 0.85f)) * v;
						const float radiusZ = collarRz * (1.0f - v) + (botW * 0.54f) * v;

						const float backComp = cosT * radiusX + backOff * (0.85f * v);
						const float sideComp = sinT * radiusZ;
						const float shoulderCurve = drop * (s * s) * (1.0f - 0.6f * v);

						pLocal = effectiveCollarOrigin
							+ backDir * backComp
							+ rightDir * sideComp
							- upDir * (rowLen + shoulderCurve);
					}
					else
					{
						// ClassicBack or Scarf: curved shoulder-hugging arc anchored to the upper-back cape attachment surface
						// so left/right clasps curve forward onto the shoulders instead of sticking out into empty air
						const float arcDeg = (rawStyle.collarStyle == CapeCollarStyle::Scarf) ? 132.0f : 110.0f;
						const float halfArcRad = (arcDeg * 0.5f) * (kPi / 180.0f);
						const float theta = s * halfArcRad;
						const float effTheta = theta * (1.0f - 0.45f * v);
						const float cosT = std::cos(effTheta);
						const float sinT = std::sin(effTheta);
						const float sinEdge = (std::max)(0.35f, std::sin(halfArcRad * (1.0f - 0.45f * v)));

						const float halfW = 0.5f * (topW + (botW - topW) * v);
						const float collarBackDepth = collarMinBackR + 0.008f * scale;
						const float radiusX = collarBackDepth + (bell * std::sin(v * kPi * 0.82f) + backOff * 0.75f) * v;
						const float sideComp = (sinT / sinEdge) * halfW;
						const float backComp = cosT * radiusX;
						const float shoulderCurve = (drop * 0.35f) * (s * s) * (1.0f - 0.5f * v);

						pLocal = effectiveCollarOrigin
							+ backDir * backComp
							+ rightDir * sideComp
							- upDir * (rowLen + shoulderCurve);
					}

					if (r > 0 && bodyCollider.enabled)
					{
						// Drape rest-pose rows naturally outside the character's canonical body/shoulders/hips/legs
						bodyCollider.CollideParticle(pLocal, nullptr, u, v, forwardDir, upDir, rightDir, 0.0f, false);
					}

					CapeParticle& pt = particles[static_cast<size_t>(r * cols + c)];
					pt.pos = pLocal;
					pt.prevPos = pLocal;
					pt.restLocalPos = pLocal;
					pt.uNorm = u;
					pt.vNorm = v;
					pt.uv = { u, v };
					pt.anchorIndex = c;

					if (r == 0)
					{
						pt.invMass = 0.0f;
						pt.tetherMaxDist = 0.0f;
						// Distribute collar pin weights smoothly across LeftShoulder, Neck, RightShoulder
						const float leftW = (s < 0.0f) ? (-s) * 0.75f : 0.0f;
						const float rightW = (s > 0.0f) ? s * 0.75f : 0.0f;
						const float neckW = 1.0f - leftW - rightW;
						pt.anchorWeights[0] = neckW;
						pt.anchorWeights[1] = leftW;
						pt.anchorWeights[2] = rightW;
						pt.anchorLocalOffsets[0] = pLocal - neckAnchor;
						pt.anchorLocalOffsets[1] = pLocal - leftShoulderAnchor;
						pt.anchorLocalOffsets[2] = pLocal - rightShoulderAnchor;
					}
					else
					{
						// Slightly heavier hem gives Sky-like smooth momentum and prevents papery jitter
						const float mass = 1.0f + 0.55f * v;
						pt.invMass = 1.0f / mass;
						const CapeVec3& topAnchor = particles[static_cast<size_t>(c)].restLocalPos;
						pt.tetherMaxDist = (pLocal - topAnchor).Length();
					}
				}
			}

			auto addConstraint = [&](int r0, int c0, int r1, int c1, float stiffness)
			{
				const uint16_t idx0 = static_cast<uint16_t>(r0 * cols + c0);
				const uint16_t idx1 = static_cast<uint16_t>(r1 * cols + c1);
				if (particles[idx0].invMass == 0.0f && particles[idx1].invMass == 0.0f)
					return;
				const float restLen = (particles[idx0].restLocalPos - particles[idx1].restLocalPos).Length();
				if (restLen > 1e-5f)
				{
					constraints.push_back({ idx0, idx1, restLen, stiffness });
				}
			};

			for (int r = 0; r < rows; ++r)
			{
				for (int c = 0; c < cols; ++c)
				{
					// Structural
					if (c + 1 < cols) addConstraint(r, c, r, c + 1, 1.0f);
					if (r + 1 < rows) addConstraint(r, c, r + 1, c, 1.0f);
					// Shear
					if (r + 1 < rows && c + 1 < cols)
					{
						addConstraint(r, c, r + 1, c + 1, 0.75f);
						addConstraint(r + 1, c, r, c + 1, 0.75f);
					}
					// Bending (2-step)
					if (c + 2 < cols) addConstraint(r, c, r, c + 2, 0.45f);
					if (r + 2 < rows) addConstraint(r, c, r + 2, c, 0.45f);
				}
			}

			triIndices.reserve(static_cast<size_t>((cols - 1) * (rows - 1) * 6));
			for (int r = 0; r < rows - 1; ++r)
			{
				for (int c = 0; c < cols - 1; ++c)
				{
					const uint16_t i00 = static_cast<uint16_t>(r * cols + c);
					const uint16_t i10 = static_cast<uint16_t>(r * cols + c + 1);
					const uint16_t i01 = static_cast<uint16_t>((r + 1) * cols + c);
					const uint16_t i11 = static_cast<uint16_t>((r + 1) * cols + c + 1);

					triIndices.push_back(i00);
					triIndices.push_back(i10);
					triIndices.push_back(i01);

					triIndices.push_back(i10);
					triIndices.push_back(i11);
					triIndices.push_back(i01);
				}
			}
			simAccumulator = 0.0f;
		}

		void ResetToAnchors(
			const CapeVec3& neckAnchor,
			const CapeVec3& leftShoulderAnchor,
			const CapeVec3& rightShoulderAnchor,
			const CapeVec3& initialNeckRef)
		{
			const CapeVec3 delta = neckAnchor - initialNeckRef;
			for (int r = 0; r < rows; ++r)
			{
				for (int c = 0; c < cols; ++c)
				{
					CapeParticle& pt = particles[static_cast<size_t>(r * cols + c)];
					if (pt.invMass == 0.0f)
					{
						pt.pos = (neckAnchor + pt.anchorLocalOffsets[0]) * pt.anchorWeights[0]
							+ (leftShoulderAnchor + pt.anchorLocalOffsets[1]) * pt.anchorWeights[1]
							+ (rightShoulderAnchor + pt.anchorLocalOffsets[2]) * pt.anchorWeights[2];
					}
					else
					{
						pt.pos = pt.restLocalPos + delta;
					}
					pt.prevPos = pt.pos;
				}
			}
			simAccumulator = 0.0f;
		}

		static void ProjectPointOutOfCapsule(CapeVec3& p, const CapeCapsule& cap, const CapeVec3* pBackDir = nullptr)
		{
			const CapeVec3 ab = cap.b - cap.a;
			const float abLenSq = ab.LengthSq();
			float t = 0.0f;
			if (abLenSq > 1e-8f)
			{
				t = (p - cap.a).Dot(ab) / abLenSq;
				t = (std::max)(0.0f, (std::min)(1.0f, t));
			}
			const CapeVec3 closest = cap.a + ab * t;
			CapeVec3 diff = p - closest;
			const float distSq = diff.LengthSq();
			const float r = cap.radius;
			if (distSq < r * r)
			{
				// Never push a cape particle forward through a leg/torso capsule; bias toward backDir
				if (pBackDir)
				{
					const float backComp = diff.Dot(*pBackDir);
					if (backComp < -r * 0.10f)
					{
						diff += (*pBackDir) * (-backComp + r * 0.55f);
					}
				}
				const float newDistSq = diff.LengthSq();
				if (newDistSq > 1e-10f)
				{
					const float dist = std::sqrt(newDistSq);
					p = closest + diff * (r / dist);
				}
				else
				{
					p = closest + (pBackDir ? (*pBackDir * r) : CapeVec3{ -r, 0.0f, 0.0f });
				}
			}
		}

		void StepSimulation(
			float dt,
			const CapeSimTuning& tuning,
			const CapeMotionInput& motion,
			const CapeVec3& neckAnchor,
			const CapeVec3& leftShoulderAnchor,
			const CapeVec3& rightShoulderAnchor,
			const CapeVec3& initialNeckRef,
			const CapeCapsule* capsules,
			size_t capsuleCount,
			int solverIterations = 3)
		{
			if (particles.empty() || cols < 2 || rows < 2)
				return;

			if (motion.teleported || !std::isfinite(dt) || dt < 0.0f)
			{
				ResetToAnchors(neckAnchor, leftShoulderAnchor, rightShoulderAnchor, initialNeckRef);
				return;
			}
			if (dt == 0.0f)
				return;

			const float clampedDt = (std::min)(dt, 0.05f);
			static constexpr float kFixedStep = 1.0f / 60.0f;
			simAccumulator = (std::min)(simAccumulator + clampedDt, kFixedStep * 3.0f);

			// Update top-row pinned anchors directly to current animated shoulder/neck positions
			for (int c = 0; c < cols; ++c)
			{
				CapeParticle& pt = particles[static_cast<size_t>(c)];
				pt.pos = (neckAnchor + pt.anchorLocalOffsets[0]) * pt.anchorWeights[0]
					+ (leftShoulderAnchor + pt.anchorLocalOffsets[1]) * pt.anchorWeights[1]
					+ (rightShoulderAnchor + pt.anchorLocalOffsets[2]) * pt.anchorWeights[2];
				pt.prevPos = pt.pos;
			}

			const CapeVec3 backDir = motion.forwardDir * -1.0f;
			const float forwardSpeed = motion.localVelocity.Dot(motion.forwardDir);
			const float horizSpeed = std::sqrt(
				motion.localVelocity.x * motion.localVelocity.x +
				motion.localVelocity.z * motion.localVelocity.z);
			const float vertSpeed = motion.localVelocity.Dot(motion.upDir);

			// Clamp inertial reaction so sudden sprints feel punchy without stretching;
			// dampen reverse (-forwardDir) drag when walking backward so cloth rests naturally against the back
			CapeVec3 clampedVel = motion.localVelocity;
			const float speedSq = clampedVel.LengthSq();
			static constexpr float kMaxInertiaSpeed = 10.0f;
			if (speedSq > kMaxInertiaSpeed * kMaxInertiaSpeed)
			{
				clampedVel *= (kMaxInertiaSpeed / std::sqrt(speedSq));
			}
			const float clampedFwd = clampedVel.Dot(motion.forwardDir);
			if (clampedFwd < 0.0f)
			{
				clampedVel -= motion.forwardDir * (clampedFwd * 0.72f);
			}
			const float clampedYawVel = (std::max)(-8.0f, (std::min)(8.0f, motion.yawAngularVel));

			const float bendGain = (std::max)(0.05f, (std::min)(1.0f, tuning.bendStiffness));
			const float maxStretch = (std::max)(1.01f, (std::min)(1.35f, tuning.stretchLimit));
			const float dampingKeep = 1.0f - (std::max)(0.005f, (std::min)(0.35f, tuning.damping));

			int substeps = 0;
			while (simAccumulator >= kFixedStep * 0.5f && substeps < 2)
			{
				const float h = (simAccumulator >= kFixedStep) ? kFixedStep : simAccumulator;
				simAccumulator -= h;
				++substeps;
				const float h2 = h * h;

				// 1. Verlet prediction + Sky-style aerodynamics
				for (size_t i = static_cast<size_t>(cols); i < particles.size(); ++i)
				{
					CapeParticle& pt = particles[i];
					if (pt.invMass <= 0.0f)
						continue;

					CapeVec3 vel = (pt.pos - pt.prevPos) * dampingKeep;

					// World-motion inertia in character local frame: moving forward pushes cloth backward
					const CapeVec3 inertiaShift = clampedVel * (-h * 0.42f * (0.35f + 0.65f * pt.vNorm));

					// Centrifugal / tangential swirl when turning or rotating preview avatar
					const CapeVec3 relFromNeck = pt.pos - neckAnchor;
					const CapeVec3 turnTangent = motion.upDir.Cross(relFromNeck);
					const CapeVec3 swirlShift = turnTangent * (-clampedYawVel * h * 0.28f * pt.vNorm);

					// Gravity (reduced slightly during flight for weightless Sky gliding feel)
					const float effGravity = motion.isFlying ? (tuning.gravity * 0.38f) : tuning.gravity;
					CapeVec3 accel = motion.upDir * (-effGravity);

					// Running / falling / flying airflow & lift
					const float runFactor = (std::min)(1.0f, horizSpeed / 5.5f);
					if (horizSpeed > 0.15f)
					{
						if (forwardSpeed >= -0.05f)
						{
							// Forward / strafe motion: push backward along backDir and lift upward at 30-55 degrees
							const float backPush = (std::max)(0.0f, forwardSpeed) * 2.2f + horizSpeed * 0.85f;
							const float lift = horizSpeed * tuning.liftFactor * (0.4f + 0.8f * pt.vNorm);
							accel += backDir * (backPush * tuning.windResponse);
							accel += motion.upDir * lift;
						}
						else
						{
							// Walking backward: inertia presses cloth onto back collider while airflow parts it gently around sides
							const float backWalkSpeed = -forwardSpeed;
							const float sideSign = (pt.uNorm - 0.5f) * 2.0f;
							accel += motion.rightDir * (sideSign * backWalkSpeed * 1.1f * tuning.windResponse * pt.vNorm);
							accel += motion.upDir * (backWalkSpeed * tuning.liftFactor * 0.30f * pt.vNorm);
						}
					}

					// Falling billows the cape upward and outward like a gentle parachute
					if (vertSpeed < -0.25f)
					{
						const float fallSpeed = (std::min)(10.0f, -vertSpeed);
						accel += motion.upDir * (fallSpeed * 0.85f * tuning.liftFactor);
						accel += backDir * (fallSpeed * 0.55f * pt.vNorm);
					}

					// Sky signature traveling wave flutter (gentle idle breeze -> energetic flight ripple)
					const float waveFreq = motion.isFlying ? 9.5f : (3.6f + 3.8f * runFactor);
					const float waveAmp = motion.isFlying
						? (4.8f * tuning.windResponse)
						: ((0.95f + 1.85f * runFactor) * tuning.windResponse);
					const float phase = pt.vNorm * 4.2f - motion.timeSeconds * waveFreq + pt.uNorm * 2.1f;
					const float secondary = std::cos(pt.uNorm * 6.2831853f - motion.timeSeconds * (waveFreq * 0.63f));
					const float envelope = pt.vNorm * pt.vNorm;
					accel += backDir * ((0.35f + 0.65f * std::sin(phase)) * waveAmp * envelope);
					accel += motion.upDir * (std::sin(phase + 0.8f) * waveAmp * 0.45f * envelope);
					accel += motion.rightDir * (secondary * waveAmp * 0.22f * envelope);

					// External wind
					accel += motion.localWind * (tuning.windResponse * (0.4f + 0.6f * pt.vNorm));

					pt.prevPos = pt.pos;
					pt.pos += vel + inertiaShift + swirlShift + accel * h2;
				}

				// 2. XPBD / PBD iterative distance & tether constraints + capsule collision
				const int iters = (std::max)(1, (std::min)(6, solverIterations));
				for (int iter = 0; iter < iters; ++iter)
				{
					for (const CapeDistanceConstraint& c : constraints)
					{
						CapeParticle& p0 = particles[c.i0];
						CapeParticle& p1 = particles[c.i1];
						const float wSum = p0.invMass + p1.invMass;
						if (wSum <= 0.0f)
							continue;

						const CapeVec3 delta = p1.pos - p0.pos;
						const float len = delta.Length();
						if (len < 1e-6f)
							continue;

						const float k = (c.stiffness < 0.6f) ? bendGain : c.stiffness;
						const float diff = (len - c.restLength) / len;
						const CapeVec3 corr = delta * (diff * k / wSum);
						p0.pos += corr * p0.invMass;
						p1.pos -= corr * p1.invMass;
					}

					// Long-range tether constraint against top collar anchor (prevents over-stretching)
					for (size_t i = static_cast<size_t>(cols); i < particles.size(); ++i)
					{
						CapeParticle& pt = particles[i];
						if (pt.invMass <= 0.0f || pt.tetherMaxDist <= 0.0f)
							continue;
						const CapeVec3& anchorPos = particles[static_cast<size_t>(pt.anchorIndex)].pos;
						const CapeVec3 d = pt.pos - anchorPos;
						const float dist = d.Length();
						const float maxAllowed = pt.tetherMaxDist * maxStretch;
						if (dist > maxAllowed && dist > 1e-6f)
						{
							pt.pos = anchorPos + d * (maxAllowed / dist);
						}
					}

					// Canonical character body envelope + animated limb capsule collision
					for (size_t i = static_cast<size_t>(cols); i < particles.size(); ++i)
					{
						CapeParticle& pt = particles[i];
						if (pt.invMass <= 0.0f)
							continue;

						if (capsules && capsuleCount > 0)
						{
							for (size_t ci = 0; ci < capsuleCount; ++ci)
							{
								ProjectPointOutOfCapsule(pt.pos, capsules[ci], &backDir);
							}
						}

						if (bodyCollider.enabled)
						{
							bodyCollider.CollideParticle(
								pt.pos,
								&pt.prevPos,
								pt.uNorm,
								pt.vNorm,
								motion.forwardDir,
								motion.upDir,
								motion.rightDir,
								horizSpeed,
								motion.isFlying);
						}
					}

					// Final iteration: prevent horizontal triangle chords from cutting across curved back/waist
					if (iter + 1 == iters && bodyCollider.enabled)
					{
						for (int r = 1; r < rows; ++r)
						{
							for (int c = 0; c + 1 < cols; ++c)
							{
								CapeParticle& pL = particles[static_cast<size_t>(r * cols + c)];
								CapeParticle& pR = particles[static_cast<size_t>(r * cols + c + 1)];
								CapeVec3 mid = (pL.pos + pR.pos) * 0.5f;
								const CapeVec3 origMid = mid;
								if (bodyCollider.CollideParticle(
									mid,
									nullptr,
									0.5f * (pL.uNorm + pR.uNorm),
									pL.vNorm,
									motion.forwardDir,
									motion.upDir,
									motion.rightDir,
									horizSpeed,
									motion.isFlying))
								{
									const CapeVec3 push = (mid - origMid) * 0.85f;
									pL.pos += push;
									pR.pos += push;
								}
							}
						}
					}
				}
			}
		}

		/** Build outer (and optional inner reverse-wound) vertices blended between rest pose and simulated pose */
		void BuildSurfaceTriangles(
			float simBlend,
			const CapeVec3& currentNeckAnchor,
			const CapeVec3& initialNeckRef,
			std::vector<CapeSurfaceVertex>& outOuterTris,
			std::vector<CapeSurfaceVertex>* outInnerTris = nullptr) const
		{
			outOuterTris.clear();
			if (outInnerTris)
				outInnerTris->clear();
			if (particles.empty() || triIndices.empty())
				return;

			const float blend = (std::max)(0.0f, (std::min)(1.0f, simBlend));
			const CapeVec3 staticOffset = currentNeckAnchor - initialNeckRef;

			std::vector<CapeVec3> blendedPos(particles.size());
			std::vector<CapeVec3> smoothNorm(particles.size(), { 0.0f, 0.0f, 0.0f });

			for (size_t i = 0; i < particles.size(); ++i)
			{
				const CapeVec3 staticPos = particles[i].restLocalPos + staticOffset;
				blendedPos[i] = (blend <= 0.001f) ? staticPos
					: ((blend >= 0.999f) ? particles[i].pos : CapeLerp(staticPos, particles[i].pos, blend));
			}

			for (size_t t = 0; t + 2 < triIndices.size(); t += 3)
			{
				const uint16_t i0 = triIndices[t + 0];
				const uint16_t i1 = triIndices[t + 1];
				const uint16_t i2 = triIndices[t + 2];
				// Left-handed DirectX/ParaEngine outward normal for front (outer back-facing) surface
				const CapeVec3 e1 = blendedPos[i1] - blendedPos[i0];
				const CapeVec3 e2 = blendedPos[i2] - blendedPos[i0];
				CapeVec3 fn = e1.Cross(e2);
				smoothNorm[i0] += fn;
				smoothNorm[i1] += fn;
				smoothNorm[i2] += fn;
			}

			for (size_t i = 0; i < smoothNorm.size(); ++i)
			{
				smoothNorm[i] = smoothNorm[i].Normalized({ -1.0f, 0.0f, 0.0f });
			}

			outOuterTris.reserve(triIndices.size());
			if (outInnerTris)
				outInnerTris->reserve(triIndices.size());

			for (size_t t = 0; t + 2 < triIndices.size(); t += 3)
			{
				const uint16_t i0 = triIndices[t + 0];
				const uint16_t i1 = triIndices[t + 1];
				const uint16_t i2 = triIndices[t + 2];

				outOuterTris.push_back({ blendedPos[i0], smoothNorm[i0], particles[i0].uv });
				outOuterTris.push_back({ blendedPos[i1], smoothNorm[i1], particles[i1].uv });
				outOuterTris.push_back({ blendedPos[i2], smoothNorm[i2], particles[i2].uv });

				if (outInnerTris)
				{
					// Reverse winding + slightly inset along inward normal to avoid Z-fighting
					static constexpr float kInnerThickness = 0.0025f;
					const CapeVec3 n0 = smoothNorm[i0] * -1.0f;
					const CapeVec3 n1 = smoothNorm[i1] * -1.0f;
					const CapeVec3 n2 = smoothNorm[i2] * -1.0f;
					outInnerTris->push_back({ blendedPos[i0] + n0 * kInnerThickness, n0, particles[i0].uv });
					outInnerTris->push_back({ blendedPos[i2] + n2 * kInnerThickness, n2, particles[i2].uv });
					outInnerTris->push_back({ blendedPos[i1] + n1 * kInnerThickness, n1, particles[i1].uv });
				}
			}
		}
	};

	/** Shortest-arc rotation matrix (3x3 row-vector convention: v' = v * R) taking unit vector 'from' to unit vector 'to' */
	struct CapeMat3
	{
		float m[3][3] = {
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f }
		};

		CapeVec3 Transform(const CapeVec3& v) const
		{
			return {
				v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0],
				v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1],
				v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2]
			};
		}

		static CapeMat3 FromTwoVectors(const CapeVec3& fromDir, const CapeVec3& toDir)
		{
			CapeMat3 r;
			const CapeVec3 f = fromDir.Normalized();
			const CapeVec3 t = toDir.Normalized();
			const float cosTheta = (std::max)(-1.0f, (std::min)(1.0f, f.Dot(t)));
			if (cosTheta > 0.99995f)
				return r;

			CapeVec3 axis = f.Cross(t);
			if (axis.LengthSq() < 1e-8f)
			{
				axis = (std::fabs(f.y) < 0.9f) ? f.Cross({ 0.0f, 1.0f, 0.0f }) : f.Cross({ 1.0f, 0.0f, 0.0f });
			}
			axis = axis.Normalized();
			const float angle = std::acos(cosTheta);
			const float c = std::cos(angle);
			const float s = std::sin(angle);
			const float omc = 1.0f - c;
			// Row-vector Rodrigues rotation (v' = v * R, right-hand rule around 'axis' = f x t)
			const float x = axis.x, y = axis.y, z = axis.z;
			r.m[0][0] = c + x * x * omc;     r.m[0][1] = x * y * omc + z * s; r.m[0][2] = x * z * omc - y * s;
			r.m[1][0] = y * x * omc - z * s; r.m[1][1] = c + y * y * omc;     r.m[1][2] = y * z * omc + x * s;
			r.m[2][0] = z * x * omc + y * s; r.m[2][1] = z * y * omc - x * s; r.m[2][2] = c + z * z * omc;
			return r;
		}
	};

	/**
	 * Frame-level distance & Top-N crowd simulation budget.
	 * Guarantees nearby players get smooth physics while distant or excess crowded characters
	 * fall back cleanly to zero-cost static rendering without popping.
	 */
	class CapeSimulationBudget
	{
	public:
		struct CandidateRecord
		{
			uintptr_t instanceId = 0;
			float distance = 0.0f;
			bool highPriority = false; // Main player or MiniSceneGraph preview actor
		};

		static CapeSimulationBudget& GetInstance()
		{
			static CapeSimulationBudget s_instance;
			return s_instance;
		}

		bool IsEnabled() const { return m_enabled; }
		void SetEnabled(bool enabled) { m_enabled = enabled; }

		bool IsAutoCapeGeosetsEnabled() const { return m_autoCapeGeosets; }
		void SetAutoCapeGeosetsEnabled(bool enabled) { m_autoCapeGeosets = enabled; }

		int GetMaxActiveCharacters() const { return m_maxActive; }
		void SetMaxActiveCharacters(int count) { m_maxActive = (std::max)(0, count); }

		float GetActivateDistance() const { return m_activateDist; }
		void SetActivateDistance(float dist)
		{
			m_activateDist = (std::max)(0.5f, dist);
			if (m_deactivateDist < m_activateDist + 1.0f)
				m_deactivateDist = m_activateDist + 1.0f;
		}

		float GetDeactivateDistance() const { return m_deactivateDist; }
		void SetDeactivateDistance(float dist)
		{
			m_deactivateDist = (std::max)(m_activateDist + 0.5f, dist);
		}

		int GetActiveSimulatedCount() const { return m_lastActiveSimulatedCount; }
		int GetEffectiveMaxActive() const { return m_effectiveMaxActive; }

		void BeginFrame(double fps, double dtSeconds)
		{
			m_lastActiveSimulatedCount = m_curFrameSimulatedCount;
			m_curFrameSimulatedCount = 0;

			// Compute cutoff distance from previous frame's candidates so Top-N nearest win deterministically
			if (!m_enabled || m_maxActive <= 0)
			{
				m_effectiveMaxActive = 0;
				m_crowdCutoffDistance = 0.0f;
				m_prevCandidates.clear();
				return;
			}

			int targetMax = m_maxActive;
			if (std::isfinite(fps) && fps > 1.0 && fps < 35.0 && std::isfinite(dtSeconds) && dtSeconds > 0.0 && dtSeconds < 0.5)
			{
				// Under heavy frame pressure, halve non-priority cape sim budget (always keep at least 2)
				targetMax = (std::max)((std::min)(m_maxActive, 2), m_maxActive / 2);
			}
			m_effectiveMaxActive = targetMax;

			if (static_cast<int>(m_prevCandidates.size()) > targetMax)
			{
				std::sort(m_prevCandidates.begin(), m_prevCandidates.end(),
					[](const CandidateRecord& a, const CandidateRecord& b)
					{
						if (a.highPriority != b.highPriority)
							return a.highPriority > b.highPriority;
						return a.distance < b.distance;
					});
				m_allowedIds.clear();
				m_allowedIds.reserve(static_cast<size_t>(targetMax));
				for (int i = 0; i < targetMax; ++i)
				{
					m_allowedIds.push_back(m_prevCandidates[static_cast<size_t>(i)].instanceId);
				}
				std::sort(m_allowedIds.begin(), m_allowedIds.end());
				m_hasCrowdFilter = true;
			}
			else
			{
				m_allowedIds.clear();
				m_hasCrowdFilter = false;
			}
			m_prevCandidates.clear();
		}

		/**
		 * Evaluates whether a character instance should run physics this frame and advances its smooth [0..1] blend factor.
		 * Returns true when simBlend > 0 (either active or smoothly blending out).
		 */
		bool EvaluateInstance(
			uintptr_t instanceId,
			float cameraDistance,
			bool isHighPriority,
			float dtSeconds,
			bool& inOutTargetActive,
			float& inOutSimBlend,
			float blendDuration = 0.25f)
		{
			if (!m_enabled || m_maxActive <= 0)
			{
				inOutTargetActive = false;
				inOutSimBlend = 0.0f;
				return false;
			}

			const float dist = std::isfinite(cameraDistance) ? (std::max)(0.0f, cameraDistance) : 9999.0f;
			const bool withinDistanceBand = inOutTargetActive
				? (dist <= m_deactivateDist)
				: (dist <= m_activateDist);

			if (withinDistanceBand || isHighPriority)
			{
				if (isHighPriority && dist <= m_deactivateDist * 1.5f)
				{
					m_prevCandidates.push_back({ instanceId, dist, true });
				}
				else if (withinDistanceBand)
				{
					m_prevCandidates.push_back({ instanceId, dist, false });
				}
			}

			bool allowedByBudget = withinDistanceBand || (isHighPriority && dist <= m_deactivateDist);
			if (allowedByBudget && !isHighPriority)
			{
				if (m_hasCrowdFilter)
				{
					allowedByBudget = std::binary_search(m_allowedIds.begin(), m_allowedIds.end(), instanceId);
				}
				else if (m_curFrameSimulatedCount >= m_effectiveMaxActive)
				{
					allowedByBudget = false;
				}
			}

			inOutTargetActive = allowedByBudget;
			const float step = (blendDuration > 1e-4f && std::isfinite(dtSeconds) && dtSeconds > 0.0f)
				? (std::min)(1.0f, dtSeconds / blendDuration)
				: (inOutTargetActive ? 1.0f : 0.0f);

			if (inOutTargetActive)
				inOutSimBlend = (std::min)(1.0f, inOutSimBlend + (dtSeconds <= 0.0f ? 1.0f : step));
			else
				inOutSimBlend = (std::max)(0.0f, inOutSimBlend - (dtSeconds <= 0.0f ? 1.0f : step));

			if (inOutSimBlend > 0.001f)
			{
				++m_curFrameSimulatedCount;
				return true;
			}
			return false;
		}

		void ResetForTest()
		{
			m_enabled = true;
			m_autoCapeGeosets = true;
			m_maxActive = 12;
			m_effectiveMaxActive = 12;
			m_activateDist = 18.0f;
			m_deactivateDist = 21.0f;
			m_curFrameSimulatedCount = 0;
			m_lastActiveSimulatedCount = 0;
			m_crowdCutoffDistance = 0.0f;
			m_hasCrowdFilter = false;
			m_prevCandidates.clear();
			m_allowedIds.clear();
		}

	private:
		bool m_enabled = true;
		bool m_autoCapeGeosets = true;
		int m_maxActive = 12;
		int m_effectiveMaxActive = 12;
		float m_activateDist = 18.0f;
		float m_deactivateDist = 21.0f;
		int m_curFrameSimulatedCount = 0;
		int m_lastActiveSimulatedCount = 0;
		float m_crowdCutoffDistance = 0.0f;
		bool m_hasCrowdFilter = false;
		std::vector<CandidateRecord> m_prevCandidates;
		std::vector<uintptr_t> m_allowedIds;
	};

	/**
	 * Procedural RGBA8888 fabric synthesizer for Sky-style DIY capes:
	 * Combines base ARGB color, vertical hem gradient, crisp dual-line trim border, subtle weave shading,
	 * and an optional glowing 4-point Sky star-wing emblem on the upper back.
	 */
	inline void SynthesizeCapePatternRGBA(
		int width,
		int height,
		uint32_t baseColorARGB,
		uint32_t gradientColorARGB,
		uint32_t trimColorARGB,
		float trimWidth,
		bool drawStarEmblem,
		uint32_t emblemColorARGB,
		float emblemScale,
		std::vector<uint8_t>& outRGBA)
	{
		const int w = (std::max)(16, width);
		const int h = (std::max)(16, height);
		outRGBA.resize(static_cast<size_t>(w * h * 4));

		auto unpack = [](uint32_t argb, float& r, float& g, float& b, float& a)
		{
			a = static_cast<float>((argb >> 24) & 0xff) / 255.0f;
			r = static_cast<float>((argb >> 16) & 0xff) / 255.0f;
			g = static_cast<float>((argb >> 8) & 0xff) / 255.0f;
			b = static_cast<float>(argb & 0xff) / 255.0f;
		};

		float br, bg, bb, ba;
		unpack(baseColorARGB, br, bg, bb, ba);
		float gr, gg, gb, ga;
		unpack(gradientColorARGB, gr, gg, gb, ga);
		float tr, tg, tb, ta;
		unpack(trimColorARGB, tr, tg, tb, ta);
		float er, eg, eb, ea;
		unpack(emblemColorARGB, er, eg, eb, ea);

		const float trimW = (std::max)(0.0f, (std::min)(0.3f, trimWidth));
		const float embSize = (std::max)(0.05f, (std::min)(0.45f, emblemScale));

		for (int y = 0; y < h; ++y)
		{
			const float v = (static_cast<float>(y) + 0.5f) / static_cast<float>(h); // 0 = collar, 1 = hem
			for (int x = 0; x < w; ++x)
			{
				const float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(w);

				float r = br, g = bg, b = bb, a = (ba > 0.001f ? ba : 1.0f);

				// Vertical gradient toward hem
				if (ga > 0.001f)
				{
					const float gt = std::pow(v, 1.35f) * ga;
					r = r + (gr - r) * gt;
					g = g + (gg - g) * gt;
					b = b + (gb - b) * gt;
				}

				// Subtle warm collar shadow + soft fabric sheen
				const float collarShade = 0.90f + 0.10f * (std::min)(1.0f, v * 5.0f);
				r *= collarShade;
				g *= collarShade;
				b *= collarShade;

				// Sky-style hem trim band + fine inner accent line
				if (trimW > 0.002f && ta > 0.001f)
				{
					const float mainBandStart = 1.0f - trimW;
					const float pinLineCenter = mainBandStart - trimW * 0.48f;
					const float pinHalfWidth = (std::max)(0.006f, trimW * 0.14f);
					float trimMask = 0.0f;
					if (v >= mainBandStart)
					{
						trimMask = (std::min)(1.0f, (v - mainBandStart) * static_cast<float>(h) * 0.5f + 0.65f);
					}
					else if (std::fabs(v - pinLineCenter) <= pinHalfWidth)
					{
						trimMask = (1.0f - std::fabs(v - pinLineCenter) / pinHalfWidth) * 0.85f;
					}
					// Subtle side-edge piping
					const float sideDist = (std::min)(u, 1.0f - u);
					if (sideDist < trimW * 0.45f)
					{
						trimMask = (std::max)(trimMask, 0.7f);
					}
					trimMask *= ta;
					r = r + (tr - r) * trimMask;
					g = g + (tg - g) * trimMask;
					b = b + (tb - b) * trimMask;
				}

				// Sky-inspired 4-point star-wing emblem on upper-mid back (u=0.5, v=0.28)
				if (drawStarEmblem && ea > 0.001f)
				{
					const float dx = (u - 0.5f) / (embSize * 0.65f);
					const float dy = (v - 0.28f) / embSize;
					// Concave 4-point star metric: |dx|^0.55 + |dy|^0.55 <= 1
					const float starDist = std::pow(std::fabs(dx), 0.58f) + std::pow(std::fabs(dy), 0.58f);
					if (starDist < 1.18f)
					{
						const float core = (starDist <= 0.82f) ? 1.0f : (std::max)(0.0f, (1.05f - starDist) / 0.23f);
						const float halo = (std::max)(0.0f, 1.0f - starDist / 1.18f) * 0.42f;
						const float m = (std::min)(1.0f, (core + halo) * ea);
						r = (std::min)(1.0f, r * (1.0f - m * 0.65f) + er * m);
						g = (std::min)(1.0f, g * (1.0f - m * 0.65f) + eg * m);
						b = (std::min)(1.0f, b * (1.0f - m * 0.65f) + eb * m);
					}
				}

				const size_t idx = static_cast<size_t>((y * w + x) * 4);
				outRGBA[idx + 0] = static_cast<uint8_t>((std::max)(0.0f, (std::min)(255.0f, r * 255.0f + 0.5f)));
				outRGBA[idx + 1] = static_cast<uint8_t>((std::max)(0.0f, (std::min)(255.0f, g * 255.0f + 0.5f)));
				outRGBA[idx + 2] = static_cast<uint8_t>((std::max)(0.0f, (std::min)(255.0f, b * 255.0f + 0.5f)));
				outRGBA[idx + 3] = static_cast<uint8_t>((std::max)(0.0f, (std::min)(255.0f, a * 255.0f + 0.5f)));
			}
		}
	}
}
