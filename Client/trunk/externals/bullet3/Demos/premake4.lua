
  
    
function createDemos( demos, incdirs, linknames)
	for _, name in ipairs(demos) do
				
			project ( "App_" .. name )
			
			kind "ConsoleApp"
			targetdir ".."

		configuration {}
	
	  	includedirs {incdirs}
		links {
                                linknames
                        }

		configuration { "Windows" }
			defines { "GLEW_STATIC"}
	 		links { "opengl32","glu32","winmm"}
			includedirs{	"Glut"	}
	 		libdirs {"Glut"}
	 		files   { "../build3/bullet.rc" }
	 		
	 		configuration {"Windows", "x32"}
				links {"glew32s","glut32"}
			configuration {"Windows", "x64"}
				links {"glew64s", "glut64"}

			
		configuration {"MacOSX"}
			--print "hello"
	 		linkoptions { "-framework Carbon -framework OpenGL -framework AGL -framework Glut" } 
		
		configuration {"not Windows", "not MacOSX"}
			links {"GL","GLU","glut","pthread"}
		configuration{}
	
		
		files     { 
		 	"./" .. name .. "/*.cpp" ,
		 	"./" .. name .. "/*.h"
		 }
	end
end

-- "CharacterDemo", fixme: it includes BspDemo files

 local localdemos = {
    "BasicDemo",
    "Box2dDemo",
    "BspDemo",
    "CcdPhysicsDemo",
    "CollisionDemo",
    "CollisionInterfaceDemo",
    "ConcaveConvexcastDemo",
    "ConcaveDemo",
    "ConcaveRaycastDemo",
    "ConstraintDemo",
    "ContinuousConvexCollision",
    "ConvexHullDistance",
    "DynamicControlDemo",
    "EPAPenDepthDemo",
    "ForkLiftDemo",
    "FeatherstoneMultiBodyDemo",
    "FractureDemo",
    "GenericJointDemo",
    "GimpactTestDemo",
    "GjkConvexCastDemo",
    "GyroscopicDemo",
    "InternalEdgeDemo",
    "MovingConcaveDemo",
    "MultiMaterialDemo",
    "RagdollDemo",
    "Raytracer",
    "RaytestDemo",
    "RollingFrictionDemo",
    "SimplexDemo",
    "SliderConstraintDemo",
    "TerrainDemo",
    "UserCollisionAlgorithm",
    "VehicleDemo",
    "VoronoiFractureDemo"
  }

-- the following demos require custom include or link settings

 createDemos({"HelloWorld"},{"../src"},{"BulletDynamics","BulletCollision","LinearMath"})

 createDemos(localdemos,{"../src","OpenGL","../btgui",},{"OpenGLSupport","BulletDynamics", "BulletCollision", "LinearMath"})
 
 createDemos({"ConvexDecompositionDemo"},{"../Extras/HACD","../Extras/ConvexDecomposition","../src","OpenGL"},{"OpenGLSupport","BulletDynamics", "BulletCollision", "LinearMath","HACD","ConvexDecomposition"})
 
 createDemos({"SoftDemo"},{"../src","OpenGL"}, {"OpenGLSupport","BulletSoftBody", "BulletDynamics", "BulletCollision", "LinearMath"})
 
 createDemos({"SerializeDemo"},{"../Extras/Serialize/BulletFileLoader","../Extras/Serialize/BulletWorldImporter","../src","OpenGL","../btgui"},{"OpenGLSupport","BulletWorldImporter", "BulletFileLoader", "BulletSoftBody", "BulletDynamics", "BulletCollision", "LinearMath"})

createDemos({"BulletXmlImportDemo"},{"../Extras/Serialize/BulletFileLoader","../Extras/Serialize/BulletXmlWorldImporter", "../Extras/Serialize/BulletWorldImporter","../src","OpenGL","btgui"},{"OpenGLSupport","BulletXmlWorldImporter","BulletWorldImporter", "BulletFileLoader", "BulletSoftBody", "BulletDynamics", "BulletCollision", "LinearMath"})
 

include "OpenGL"
 
 
