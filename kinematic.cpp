#include "kinematic.h"

//! Make conversion easy:
#define toRadians (M_PI/180.0)
#define toDegrees (180.0/M_PI)

//! Kdl global data storage definition:
KDL::Chain KDLChain;
KDL::Frame cart, cartzero;
KDL::JntArray KDLJointInit;
KDL::JntArray KDLJointCur;
KDL::JntArray KDLJointMin;
KDL::JntArray KDLJointMax;

int kinematic::Init(){
    // =========================================================================
    // CRITICAL FIX: Reset the global chain to empty before adding segments.
    // =========================================================================
    KDLChain = KDL::Chain();

    //! Setup Kdl chain based on the new relative differences (converted to mm)
    // J0 (Base to J1): 155.0 - 0 = 155.0 | 470.0 - 0 = 470.0
    KDLChain.addSegment(KDL::Segment("J0",KDL::Joint(KDL::Joint::RotZ), KDL::Frame(KDL::Vector(155.0, 0.0, 470.0))));

    // J1 (J1 to J2): 1074.0 - 470.0 = 604.0
    KDLChain.addSegment(KDL::Segment("J1",KDL::Joint(KDL::Joint::RotY), KDL::Frame(KDL::Vector(0.0, 0.0, 604.0))));

    // J2 (J2 to J3): 1274.0 - 1074.0 = 200.0
    KDLChain.addSegment(KDL::Segment("J2",KDL::Joint(KDL::Joint::RotY), KDL::Frame(KDL::Vector(0.0, 0.0, 200.0))));

    // J3 (J3 to J4): 795.5 - 155.0 = 640.5
    KDLChain.addSegment(KDL::Segment("J3",KDL::Joint(KDL::Joint::RotX), KDL::Frame(KDL::Vector(640.5, 0.0, 0.0))));

    // J4 (Wrist Pitch - Intersects with J5 at the wrist center)
    KDLChain.addSegment(KDL::Segment("J4",KDL::Joint(KDL::Joint::RotY), KDL::Frame(KDL::Vector(0.0, 0.0, 0.0))));

    // J5 (Wrist Roll to Tool Flange): 895.5 - 795.5 = 100.0
    KDLChain.addSegment(KDL::Segment("J5",KDL::Joint(KDL::Joint::RotX), KDL::Frame(KDL::Vector(100.0, 0.0, 0.0))));

    KDLJointMin.resize(KDLChain.getNrOfSegments());
    KDLJointMax.resize(KDLChain.getNrOfSegments());
    KDLJointCur.resize(KDLChain.getNrOfSegments());
    KDLJointInit.resize(KDLChain.getNrOfSegments());

    // Default Limits
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

    if(Fk()){
        return 1;
    } else {
        std::cout << "[ERROR] Forward kinematics initialization failed." << std::endl;
        return 0;
    }
}




int kinematic::Fk(){
    KDL::ChainFkSolverPos_recursive fksolver(KDLChain);
    int status = fksolver.JntToCart(KDLJointCur, cart, -1);
    return (status >= 0) ? 1 : 0;
}

int kinematic::Fk_zero(){
    KDL::ChainFkSolverPos_recursive fksolver(KDLChain);
    int status = fksolver.JntToCart(KDLJointInit, cartzero, -1);
    return (status >= 0) ? 1 : 0;
}

int kinematic::Ik(){
    KDL::ChainFkSolverPos_recursive fksolver(KDLChain);
    KDL::ChainIkSolverVel_pinv iksolverv(KDLChain);
    KDL::ChainIkSolverPos_NR_JL iksolver(KDLChain, KDLJointMin, KDLJointMax, fksolver, iksolverv, 100, 1e-6);

    KDL::JntArray JntResult(KDLChain.getNrOfJoints());
    int status = mode_ikfrominit ? iksolver.CartToJnt(KDLJointInit, cart, JntResult)
                                 : iksolver.CartToJnt(KDLJointCur, cart, JntResult);

    if(status >= 0){
        KDLJointCur = JntResult;
        return 1;
    }
    return 0;
}

int kinematic::temp_Ik(bool ikfrominit, KDL::Chain chain, KDL::JntArray jointinit, KDL::JntArray jointmin, KDL::JntArray jointmax, KDL::Frame target_cart, KDL::JntArray &jointcur){
    KDL::ChainFkSolverPos_recursive fksolver(chain);
    KDL::ChainIkSolverVel_pinv iksolverv(chain);
    KDL::ChainIkSolverPos_NR_JL iksolver(chain, jointmin, jointmax, fksolver, iksolverv, 100, 1e-6);

    KDL::JntArray JntResult(chain.getNrOfJoints());
    int status = ikfrominit ? iksolver.CartToJnt(jointinit, target_cart, JntResult)
                            : iksolver.CartToJnt(jointcur, target_cart, JntResult);

    if(status >= 0){
        jointcur = JntResult;
        return 1;
    }
    return 0;
}

int kinematic::Fk_tooldir(double x_in, double y_in, double z_in, double &x_out, double &y_out, double &z_out){
    KDL::JntArray Tooljoints;
    KDL::Frame Toolcart;
    KDL::Chain ToolChain;
    ToolChain.addChain(KDLChain);
    ToolChain.addSegment(KDL::Segment("Tooldir",KDL::Joint(KDL::Joint::RotX), KDL::Frame(KDL::Vector(x_in,y_in,z_in))));

    Tooljoints.resize(ToolChain.getNrOfSegments());
    for(unsigned int i=0; i<6; i++){
        Tooljoints(i) = KDLJointCur(i);
    }
    Tooljoints(6) = 0;

    KDL::ChainFkSolverPos_recursive fksolver = KDL::ChainFkSolverPos_recursive(ToolChain);
    int status = fksolver.JntToCart(Tooljoints,Toolcart,-1);

    if(status >= 0){
        x_out = Toolcart.p.x();
        y_out = Toolcart.p.y();
        z_out = Toolcart.p.z();
        return 1;
    } else {
        std::cout << fksolver.getError() << std::endl;
        return 0;
    }
}

int kinematic::Ik_Find_All_Solutions(const KDL::Frame &target_cart, std::vector<KDL::JntArray> &found_solutions) {
    found_solutions.clear();

    KDL::ChainFkSolverPos_recursive fksolver(KDLChain);
    KDL::ChainIkSolverVel_pinv iksolverv(KDLChain);
    // Increased max iter and loosened tolerance slightly to allow numerical solver to finish bounds
    KDL::ChainIkSolverPos_NR_JL iksolver(KDLChain, KDLJointMin, KDLJointMax, fksolver, iksolverv, 500, 1e-4);

    std::vector<KDL::JntArray> seeds;

    // Generate a systematic grid of seeds for the dominant joints (J1, J2, J4)
    // to force exploration of all Left/Right, Up/Down, Flip/No-Flip configurations.
    double j0_opts[] = {0.0, M_PI/2, -M_PI/2, M_PI};
    double j1_opts[] = {KDLJointMin(1)*0.5, 0.0, KDLJointMax(1)*0.5};
    double j2_opts[] = {KDLJointMin(2)*0.5, 0.0, KDLJointMax(2)*0.5};
    double j4_opts[] = {KDLJointMin(4)*0.5, 0.0, KDLJointMax(4)*0.5};

    for(double j0 : j0_opts) {
        for(double j1 : j1_opts) {
            for(double j2 : j2_opts) {
                for(double j4 : j4_opts) {
                    KDL::JntArray seed(6);
                    seed(0) = j0;
                    seed(1) = j1;
                    seed(2) = j2;
                    seed(3) = 0.0; // Roll joints are less critical for initial structural seeds
                    seed(4) = j4;
                    seed(5) = 0.0;
                    seeds.push_back(seed);
                }
            }
        }
    }

    for (auto& seed : seeds) {
        KDL::JntArray result(6);
        // =========================================================================
        // CRITICAL FIX: We now use target_cart directly instead of the global `cart`
        // =========================================================================
        if (iksolver.CartToJnt(seed, target_cart, result) >= 0) {

            // STRICT VERIFICATION: Does this IK result actually yield the target pose?
            KDL::Frame fk_check;
            fksolver.JntToCart(result, fk_check);
            KDL::Twist diff = KDL::diff(target_cart, fk_check);

            // If the position error is > 1mm or rotation error is > 0.01 rad, it's a false local minimum. Reject it.
            if (diff.vel.Norm() > 1.0 || diff.rot.Norm() > 0.01) {
                continue;
            }

            // Check for duplicates
            bool is_unique = true;
            for (auto& existing_sol : found_solutions) {
                double max_diff = 0;
                for(int j=0; j<6; j++) {
                    double d = std::abs(existing_sol(j) - result(j));
                    if (d > max_diff) max_diff = d;
                }
                // If all joints are within ~0.5 degrees, it's the same configuration
                if (max_diff < 0.01) {
                    is_unique = false;
                    break;
                }
            }

            if (is_unique) {
                found_solutions.push_back(result);
            }
        }
    }
    return found_solutions.size();
}

bool kinematic::Ik_Optimal_Solution(const KDL::Frame &target_cart, const KDL::JntArray &current_joints, KDL::JntArray &optimal_joints) {
    std::vector<KDL::JntArray> all_solutions;

    // =========================================================================
    // CRITICAL FIX: Removed `cart = target_cart;`.
    // Writing to `cart` here silently corrupted the live robot's position in memory.
    // =========================================================================

    int num_solutions = Ik_Find_All_Solutions(target_cart, all_solutions);

    if (num_solutions == 0) {
        return false; // Target unreachable or out of joint limits
    }

    double min_total_movement = 1e9; // Start with a very high threshold
    int best_index = 0;

    for (int i = 0; i < num_solutions; ++i) {
        double total_movement = 0.0;

        // Calculate the sum of absolute angular differences for all 6 joints
        for (int j = 0; j < 6; ++j) {
            total_movement += std::abs(all_solutions[i](j) - current_joints(j));
        }

        // Select the solution with the minimal movement
        if (total_movement < min_total_movement) {
            min_total_movement = total_movement;
            best_index = i;
        }
    }

    optimal_joints = all_solutions[best_index];
    return true;
}

void kinematic::UpdateLimits(double j1mn, double j1mx, double j2mn, double j2mx,
                             double j3mn, double j3mx, double j4mn, double j4mx,
                             double j5mn, double j5mx, double j6mn, double j6mx)
{
    // Update the KDL Arrays used by the IK Solver in real-time
    KDLJointMin(0) = j1mn * toRadians; KDLJointMax(0) = j1mx * toRadians;
    KDLJointMin(1) = j2mn * toRadians; KDLJointMax(1) = j2mx * toRadians;
    KDLJointMin(2) = j3mn * toRadians; KDLJointMax(2) = j3mx * toRadians;
    KDLJointMin(3) = j4mn * toRadians; KDLJointMax(3) = j4mx * toRadians;
    KDLJointMin(4) = j5mn * toRadians; KDLJointMax(4) = j5mx * toRadians;
    KDLJointMin(5) = j6mn * toRadians; KDLJointMax(5) = j6mx * toRadians;
}