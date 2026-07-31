#include "kinematic.h"
#include "ur_kinematics.h"

//! Make conversion easy:
#define toRadians (M_PI/180.0)
#define toDegrees (180.0/M_PI)

static bool g_isCobotMode = false;

//! Kdl global data storage definition:
KDL::Chain KDLChain;
KDL::Frame cart, cartzero;
KDL::JntArray KDLJointInit;
KDL::JntArray KDLJointCur;
KDL::JntArray KDLJointMin;
KDL::JntArray KDLJointMax;

int kinematic::Init(){
    g_isCobotMode = false;
    KDLChain = KDL::Chain();

    KDLChain.addSegment(KDL::Segment("J0",KDL::Joint(KDL::Joint::RotZ), KDL::Frame(KDL::Vector(155.0, 0.0, 470.0))));
    KDLChain.addSegment(KDL::Segment("J1",KDL::Joint(KDL::Joint::RotY), KDL::Frame(KDL::Vector(0.0, 0.0, 604.0))));
    KDLChain.addSegment(KDL::Segment("J2",KDL::Joint(KDL::Joint::RotY), KDL::Frame(KDL::Vector(0.0, 0.0, 200.0))));
    KDLChain.addSegment(KDL::Segment("J3",KDL::Joint(KDL::Joint::RotX), KDL::Frame(KDL::Vector(640.5, 0.0, 0.0))));
    KDLChain.addSegment(KDL::Segment("J4",KDL::Joint(KDL::Joint::RotY), KDL::Frame(KDL::Vector(0.0, 0.0, 0.0))));
    KDLChain.addSegment(KDL::Segment("J5",KDL::Joint(KDL::Joint::RotX), KDL::Frame(KDL::Vector(100.0, 0.0, 0.0))));

    KDLJointMin.resize(KDLChain.getNrOfJoints());
    KDLJointMax.resize(KDLChain.getNrOfJoints());
    KDLJointCur.resize(KDLChain.getNrOfJoints());
    KDLJointInit.resize(KDLChain.getNrOfJoints());

    KDLJointMin(0) = -170 * toRadians; KDLJointMax(0) =  170 * toRadians;
    KDLJointMin(1) = -100 * toRadians; KDLJointMax(1) =  135 * toRadians;
    KDLJointMin(2) = -210 * toRadians; KDLJointMax(2) =   66 * toRadians;
    KDLJointMin(3) = -185 * toRadians; KDLJointMax(3) =  185 * toRadians;
    KDLJointMin(4) = -120 * toRadians; KDLJointMax(4) =  120 * toRadians;
    KDLJointMin(5) = -350 * toRadians; KDLJointMax(5) =  350 * toRadians;

    for(unsigned int i=0; i<6; i++){
        KDLJointInit(i) = 0.0;
        KDLJointCur(i)  = 0.0;
    }

    return Fk() ? 1 : 0;
}

int kinematic::Fk(){
    if (g_isCobotMode) {
        // 🚀 STRICT PURE ANALYTICAL FK FOR COBOT (NO KDL)
        double q[6] = {
            KDLJointCur(0),
            KDLJointCur(1) - M_PI_2,
            KDLJointCur(2),
            KDLJointCur(3) - M_PI_2,
            KDLJointCur(4),
            KDLJointCur(5)
        };
        double T[16];
        ur::forward(q, T);

        // Compute the "Mechanical Zero" offset so the UI reads A=0, B=0, C=0 at J=0
        double q_home[6] = { 0.0, -M_PI_2, 0.0, -M_PI_2, 0.0, 0.0 };
        double T_home[16];
        ur::forward(q_home, T_home);
        KDL::Rotation R_home(
            T_home[0], T_home[1], T_home[2],
            T_home[4], T_home[5], T_home[6],
            T_home[8], T_home[9], T_home[10]
            );

        KDL::Rotation R_ur(
            T[0], T[1], T[2],
            T[4], T[5], T[6],
            T[8], T[9], T[10]
            );

        // Apply the exact rotation offset to zero out ABC at home
        cart.M = R_ur * R_home.Inverse();
        cart.p.x( T[3] * 1000.0 );
        cart.p.y( T[7] * 1000.0 );
        cart.p.z( T[11] * 1000.0 );
        return 1;
    } else {
        KDL::ChainFkSolverPos_recursive fksolver(KDLChain);
        int status = fksolver.JntToCart(KDLJointCur, cart, -1);
        return (status >= 0) ? 1 : 0;
    }
}

int kinematic::Fk_zero(){
    if (g_isCobotMode) {
        double q[6] = {
            KDLJointInit(0),
            KDLJointInit(1) - M_PI_2,
            KDLJointInit(2),
            KDLJointInit(3) - M_PI_2,
            KDLJointInit(4),
            KDLJointInit(5)
        };
        double T[16];
        ur::forward(q, T);

        double q_home[6] = { 0.0, -M_PI_2, 0.0, -M_PI_2, 0.0, 0.0 };
        double T_home[16];
        ur::forward(q_home, T_home);
        KDL::Rotation R_home(
            T_home[0], T_home[1], T_home[2],
            T_home[4], T_home[5], T_home[6],
            T_home[8], T_home[9], T_home[10]
            );

        KDL::Rotation R_ur(
            T[0], T[1], T[2],
            T[4], T[5], T[6],
            T[8], T[9], T[10]
            );

        cartzero.M = R_ur * R_home.Inverse();
        cartzero.p.x( T[3] * 1000.0 );
        cartzero.p.y( T[7] * 1000.0 );
        cartzero.p.z( T[11] * 1000.0 );
        return 1;
    } else {
        KDL::ChainFkSolverPos_recursive fksolver(KDLChain);
        int status = fksolver.JntToCart(KDLJointInit, cartzero, -1);
        return (status >= 0) ? 1 : 0;
    }
}

int kinematic::Ik(){
    KDL::ChainIkSolverPos_LMA iksolver(KDLChain, 1e-5, 500, 1e-15);
    KDL::JntArray JntResult(KDLChain.getNrOfJoints());
    int status = mode_ikfrominit ? iksolver.CartToJnt(KDLJointInit, cart, JntResult)
                                 : iksolver.CartToJnt(KDLJointCur, cart, JntResult);
    if(status >= 0){
        KDLJointCur = JntResult;
        return 1;
    }
    return 0;
}

int kinematic::RebuildChain(double bx, double bz, double az, double ez, double fx, double wx)
{
    g_isCobotMode = false;
    KDLChain = KDL::Chain();
    KDLChain.addSegment(KDL::Segment("J0", KDL::Joint(KDL::Joint::RotZ), KDL::Frame(KDL::Vector(bx, 0.0, bz))));
    KDLChain.addSegment(KDL::Segment("J1", KDL::Joint(KDL::Joint::RotY), KDL::Frame(KDL::Vector(0.0, 0.0, az))));
    KDLChain.addSegment(KDL::Segment("J2", KDL::Joint(KDL::Joint::RotY), KDL::Frame(KDL::Vector(0.0, 0.0, ez))));
    KDLChain.addSegment(KDL::Segment("J3", KDL::Joint(KDL::Joint::RotX), KDL::Frame(KDL::Vector(fx, 0.0, 0.0))));
    KDLChain.addSegment(KDL::Segment("J4", KDL::Joint(KDL::Joint::RotY), KDL::Frame(KDL::Vector(0.0, 0.0, 0.0))));
    KDLChain.addSegment(KDL::Segment("J5", KDL::Joint(KDL::Joint::RotX), KDL::Frame(KDL::Vector(wx, 0.0, 0.0))));

    KDLJointMin.resize(6); KDLJointMax.resize(6); KDLJointCur.resize(6); KDLJointInit.resize(6);
    for(unsigned int i=0; i<6; i++) KDLJointInit(i) = 0.0;
    return Fk() ? 1 : 0;
}


void kinematic::UpdateLimits(double j1mn, double j1mx, double j2mn, double j2mx,
                             double j3mn, double j3mx, double j4mn, double j4mx,
                             double j5mn, double j5mx, double j6mn, double j6mx)
{
    KDLJointMin(0) = j1mn * toRadians; KDLJointMax(0) = j1mx * toRadians;
    KDLJointMin(1) = j2mn * toRadians; KDLJointMax(1) = j2mx * toRadians;
    KDLJointMin(2) = j3mn * toRadians; KDLJointMax(2) = j3mx * toRadians;
    KDLJointMin(3) = j4mn * toRadians; KDLJointMax(3) = j4mx * toRadians;
    KDLJointMin(4) = j5mn * toRadians; KDLJointMax(4) = j5mx * toRadians;
    KDLJointMin(5) = j6mn * toRadians; KDLJointMax(5) = j6mx * toRadians;
}

int kinematic::RebuildCobotChain(double bx, double bz, double az, double ez, double fx, double wx, double fy)
{
    g_isCobotMode = true;

    // 🚀 USE THE EXACT BLUEPRINT DIMENSIONS:
    ur::setCustomModel(140.15/1000.0, -430.0/1000.0, -368.5/1000.0, 144.15/1000.0, 113.5/1000.0, 109.0/1000.0);

    // Provide a dummy DH structure just to keep UI arrays alive without crashing KDL
    KDLChain = KDL::Chain();
    KDLChain.addSegment(KDL::Segment("J1", KDL::Joint(KDL::Joint::RotZ), KDL::Frame::DH(0.0, M_PI_2, 140.15, 0.0)));
    KDLChain.addSegment(KDL::Segment("J2", KDL::Joint(KDL::Joint::RotZ), KDL::Frame::DH(430.0, 0.0, 0.0, 0.0)));
    KDLChain.addSegment(KDL::Segment("J3", KDL::Joint(KDL::Joint::RotZ), KDL::Frame::DH(368.5, 0.0, 0.0, 0.0)));
    KDLChain.addSegment(KDL::Segment("J4", KDL::Joint(KDL::Joint::RotZ), KDL::Frame::DH(0.0, M_PI_2, 144.15, 0.0)));
    KDLChain.addSegment(KDL::Segment("J5", KDL::Joint(KDL::Joint::RotZ), KDL::Frame::DH(0.0, -M_PI_2, 113.5, 0.0)));
    KDLChain.addSegment(KDL::Segment("J6", KDL::Joint(KDL::Joint::RotZ), KDL::Frame::DH(0.0, 0.0, 109.0, 0.0)));

    KDLJointMin.resize(6); KDLJointMax.resize(6); KDLJointCur.resize(6); KDLJointInit.resize(6);
    for(unsigned int i=0; i<6; i++){
        KDLJointMin(i) = -360 * toRadians;
        KDLJointMax(i) =  360 * toRadians;
        KDLJointInit(i) = 0.0;
    }
    return Fk() ? 1 : 0;
}