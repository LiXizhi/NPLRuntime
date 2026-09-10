#include "ParaScriptingMuJoCo.h"
#include "ONNXPolicySession.h"
#include <cmath>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <vector>

static void check(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

int main() {
    try {
        using namespace ParaScripting;
        ONNXPolicySession policy;
        std::vector<float> observation(61, 0), action;
        observation[5] = -1;
        for (const char* name : {"walking", "stand", "sitstand", "ground-pick", "kick-left", "kick-right", "roulade"}) {
            check(policy.Load(std::string("/microduck/models/") + name + ".onnx", 61, 14), policy.GetLastError());
            check(policy.Infer(observation, action), policy.GetLastError());
            check(action.size() == 14, "wrong action size");
            std::printf("POLICY %s first_action=%.9g\n", name, action[0]);
            auto invalid = observation;
            invalid[0] = std::numeric_limits<float>::quiet_NaN();
            check(!policy.Infer(invalid, action), "NaN observation was accepted");
            check(!policy.Infer(std::vector<float>(60), action), "wrong observation width was accepted");
            policy.ResetState();
            policy.Unload(); policy.Unload();
        }
        check(!policy.Load("/missing.onnx", 61, 14), "missing policy was accepted");
        check(policy.Load("/microduck/models/stand.onnx", 61, 14), policy.GetLastError());
        MuJoCoSimulation sim;
        check(sim.Load("/microduck/mujoco/scene_ball.xml"), sim.GetLastError());
        const char* joints[] = {"left_hip_yaw", "left_hip_roll", "left_hip_pitch", "left_knee", "left_ankle", "neck_pitch", "head_pitch", "head_yaw", "head_roll", "right_hip_yaw", "right_hip_roll", "right_hip_pitch", "right_knee", "right_ankle"};
        const double rest[] = {0, -0.08726646259971647, -0.457924, -0.00494, 0.452984, 0.3490658503988659, 0.3490658503988659, 0, 0, 0, 0.08726646259971647, 0.457924, 0.00494, -0.452984};
        int qpos[14], dof[14], ctrl[14];
        for (int i=0; i<14; ++i) {
            const int joint = sim.FindJoint(joints[i]);
            ctrl[i] = sim.FindActuator(joints[i]);
            check(joint >= 0 && ctrl[i] >= 0, "joint/actuator missing");
            qpos[i] = sim.GetJointQPosAdr(joint); dof[i] = sim.GetJointDofAdr(joint);
            sim.SetQPos(qpos[i], rest[i]); sim.SetControl(ctrl[i], rest[i]);
        }
        const int trunk = sim.FindBody("trunk_base"), gyro = sim.FindSensor("imu_ang_vel");
        check(trunk >= 0 && gyro >= 0 && sim.GetSensorDim(gyro)==3, "body/gyro missing");
        sim.Forward();
        action.assign(14, 0);
        for (int tick=0; tick<1250; ++tick) {
            const double w=sim.GetBodyQuaternion(trunk,0), x=sim.GetBodyQuaternion(trunk,1), y=sim.GetBodyQuaternion(trunk,2), z=sim.GetBodyQuaternion(trunk,3);
            for(int i=0;i<3;++i) observation[i]=sim.GetSensorData(gyro,i);
            observation[3]=2*(w*y-x*z); observation[4]=-2*(w*x+y*z); observation[5]=-(1-2*(x*x+y*y));
            for(int i=0;i<14;++i) {
                observation[6+i]=sim.GetQPos(qpos[i])-rest[i];
                observation[20+i]=sim.GetQVel(dof[i]); observation[34+i]=action[i];
            }
            check(policy.Infer(observation,action),policy.GetLastError());
            for(int i=0;i<14;++i) check(sim.SetControl(ctrl[i],rest[i]+action[i]),"control write failed");
            sim.Step(4);
            for(int i=0;i<sim.GetQPosCount();++i) check(std::isfinite(sim.GetQPos(i)),"nonfinite physics state");
        }
        check(std::abs(sim.GetTime()-25)<1e-7,"physics timestep mismatch");
        const double simulationTime=sim.GetTime();
        check(sim.InitializeBlockCollisionPool(8),sim.GetLastError());
        check(std::abs(sim.GetTime()-simulationTime)<1e-9,"pool lost simulation state");
        check(sim.UpdateBlockCollisionPool("",1.2,0.0003),sim.GetLastError());
        std::printf("SIM steps=5000 time=%.6f contacts=%d trunk_z=%.6f\n",sim.GetTime(),sim.GetContactCount(),sim.GetBodyPosition(trunk,2));
        sim.Reset(); check(sim.GetTime()==0,"reset did not clear time");
        check(!sim.Load("/missing.xml"),"missing model was accepted");
        std::puts("PARAROBOT_WASM_SMOKE_PASS");
        return 0;
    } catch(const std::exception& error) {
        std::fprintf(stderr,"PARAROBOT_WASM_SMOKE_FAIL: %s\n",error.what());
        return 1;
    }
}
