#include "scurve.h"
#include <cmath>
#include <algorithm>

scurve::scurve()
{
}

std::vector<scurve::point> scurve::create_point_for_every_ms_path(
    double velmax, double accmax, double vo, double ve,
    std::vector<scurve::point> pathvec)
{
    std::vector<scurve::point> full_trajectory;
    if (pathvec.size() < 2) return full_trajectory;

    for (size_t i = 0; i < pathvec.size() - 1; ++i) {

        double current_vo = vo;
        double current_ve = 0.0;

        if (i == pathvec.size() - 2) {
            current_ve = ve; // last segment
        } else {
            // Full 3D vectors
            double v1_x = pathvec[i+1].x - pathvec[i].x;
            double v1_y = pathvec[i+1].y - pathvec[i].y;
            double v1_z = pathvec[i+1].z - pathvec[i].z;

            double v2_x = pathvec[i+2].x - pathvec[i+1].x;
            double v2_y = pathvec[i+2].y - pathvec[i+1].y;
            double v2_z = pathvec[i+2].z - pathvec[i+1].z;

            double dot_product = (v1_x*v2_x) + (v1_y*v2_y) + (v1_z*v2_z);
            double mag1 = std::sqrt(v1_x*v1_x + v1_y*v1_y + v1_z*v1_z);
            double mag2 = std::sqrt(v2_x*v2_x + v2_y*v2_y + v2_z*v2_z);

            if (mag1 < 1e-9 || mag2 < 1e-9) {
                current_ve = 0.0;
            } else {
                double cos_angle = dot_product / (mag1 * mag2);
                cos_angle = std::max(-1.0, std::min(1.0, cos_angle)); // clamp
                double angle = std::acos(cos_angle);
                double corner_velocity = velmax * (1.0 - (angle / M_PI));
                current_ve = std::max(0.0, std::min(corner_velocity, velmax));
            }
        }

        std::vector<scurve::point> segment = scurve_create_point_for_every_ms(
            velmax, accmax, current_vo, current_ve,
            pathvec[i].x,   pathvec[i].y,   pathvec[i].z,
            pathvec[i+1].x, pathvec[i+1].y, pathvec[i+1].z
            );

        vo = current_ve;

        // Safely append to the LOCAL array (Fixes the doubling bug!)
        size_t start_idx = (i == 0) ? 0 : 1;
        for (size_t j = start_idx; j < segment.size(); ++j) {
            full_trajectory.push_back(segment[j]);
        }
    }

    return full_trajectory;
}


std::vector<scurve::point> scurve::scurve_create_point_for_every_ms(double velmax, double accmax, double vo, double ve,
                                                                    double xs, double ys, double zs,
                                                                    double xe, double ye, double ze){
    pointvec.clear();
    linelenght = sqrt((xe-xs)*(xe-xs) + (ye-ys)*(ye-ys) + (ze-zs)*(ze-zs));

    std::vector<waypoint> wpvec;
    waypoint wp;
    wp.type="line";
    wp.xs=xs; wp.ys=ys; wp.zs=zs;
    wp.xe=xe; wp.ye=ye; wp.ze=ze;
    wp.lenght = linelenght;
    wpvec.push_back(wp);

    traject traject;
    traject = TrajectCalculator(velmax, accmax, vo, ve, wpvec);

    int expected_size = std::ceil(traject.Ttot * 1000.0) + 1;
    if (expected_size > 0) {
        pointvec.reserve(expected_size);
    }

    double dir_x = (linelenght > 0) ? (xe - xs) / linelenght : 0;
    double dir_y = (linelenght > 0) ? (ye - ys) / linelenght : 0;
    double dir_z = (linelenght > 0) ? (ze - zs) / linelenght : 0;

    double S_phase1_up = traject.Vo * traject.t1_up + traject.Jm * std::pow(traject.t1_up, 3) / 6.0;
    double S_phase2_up = S_phase1_up + traject.v1_up * traject.tlin_up + traject.as_up * std::pow(traject.tlin_up, 2) * 0.5;

    double S_base_down = traject.L1 + traject.L2;
    double S_phase1_down = S_base_down + traject.Vel * traject.t1_down - traject.Jm * std::pow(traject.t1_down, 3) / 6.0;
    double S_phase2_down = S_phase1_down + traject.v1_down * traject.tlin_down - traject.as_down * std::pow(traject.tlin_down, 2) * 0.5;

    // Evaluate exact analytical math over the entire continuous time
    for (int i = 0; i <= expected_size; ++i) {
        double T = i * 0.001;
        if (T > traject.Ttot) T = traject.Ttot;

        double S = 0, V = 0, A = 0, J = 0; // Added J here

        if (T <= traject.T1) {
            double t = T;
            if (t <= traject.t1_up) {
                A = traject.Jm * t;
                V = traject.Vo + 0.5 * traject.Jm * t * t;
                S = traject.Vo * t + traject.Jm * t * t * t / 6.0;
                J = traject.Jm; // Analytical Jerk
            } else if (t <= traject.t1_up + traject.tlin_up) {
                double dt = t - traject.t1_up;
                A = traject.as_up;
                V = traject.v1_up + traject.as_up * dt;
                S = S_phase1_up + traject.v1_up * dt + 0.5 * traject.as_up * dt * dt;
                J = 0.0; // Analytical Jerk
            } else {
                double dt = t - (traject.t1_up + traject.tlin_up);
                A = traject.as_up - traject.Jm * dt;
                V = traject.v2_up + traject.as_up * dt - 0.5 * traject.Jm * dt * dt;
                S = S_phase2_up + traject.v2_up * dt + 0.5 * traject.as_up * dt * dt - traject.Jm * dt * dt * dt / 6.0;
                J = -traject.Jm; // Analytical Jerk
            }
        } else if (T <= traject.T1 + traject.T2) {
            double dt = T - traject.T1;
            A = 0;
            V = traject.Vel;
            S = traject.L1 + traject.Vel * dt;
            J = 0.0; // Analytical Jerk
        } else {
            double t = T - (traject.T1 + traject.T2);
            if (t <= traject.t1_down) {
                A = -traject.Jm * t;
                V = traject.Vel - 0.5 * traject.Jm * t * t;
                S = S_base_down + traject.Vel * t - traject.Jm * t * t * t / 6.0;
                J = -traject.Jm; // Analytical Jerk
            } else if (t <= traject.t1_down + traject.tlin_down) {
                double dt = t - traject.t1_down;
                A = -traject.as_down;
                V = traject.v1_down - traject.as_down * dt;
                S = S_phase1_down + traject.v1_down * dt - 0.5 * traject.as_down * dt * dt;
                J = 0.0; // Analytical Jerk
            } else {
                double dt = t - (traject.t1_down + traject.tlin_down);
                A = -traject.as_down + traject.Jm * dt;
                V = traject.v2_down - traject.as_down * dt + 0.5 * traject.Jm * dt * dt;
                S = S_phase2_down + traject.v2_down * dt - 0.5 * traject.as_down * dt * dt + traject.Jm * dt * dt * dt / 6.0;
                J = traject.Jm; // Analytical Jerk
            }
        }

        point p;
        p.x = xs + S * dir_x;
        p.y = ys + S * dir_y;
        p.z = zs + S * dir_z;
        p.v = V;
        p.a = A;
        p.j = J; // Assign exact jerk to the point
        pointvec.push_back(p);

        if (T >= traject.Ttot) break;
    }

    return pointvec;
}

double scurve::TrajectLenght(std::vector<waypoint> pv){
    double ltot=0;
    for(unsigned int i=0; i<pv.size(); i++){
        ltot+= pv.at(i).lenght;
    }
    return ltot;
}

scurve::waypoint scurve::LineLenght(waypoint p){
    if(p.type=="line"){
        p.lenght=sqrt((p.xe-p.xs)*(p.xe-p.xs) + (p.ye-p.ys)*(p.ye-p.ys) + (p.ze-p.zs)*(p.ze-p.zs));
    }
    return p;
}

scurve::point scurve::LineLineIntersect(lines a,lines b){
    point p={0,0,0};
    double delta_y0 = a.ye-a.ys;
    double delta_x0 = a.xs-a.xe;
    double c0 = delta_y0 * a.xs + delta_x0 * a.ys;
    double delta_y1 = b.ye-b.ys;
    double delta_x1 = b.xs-b.xe;
    double c1 = delta_y1 * b.xs + delta_x1 * b.ys;
    double determinant = delta_y0*delta_x1 - delta_y1*delta_x0;
    if (determinant != 0){
        p.x = (delta_x1*c0 - delta_x0*c1)/determinant;
        p.y = (delta_y0*c1 - delta_y1*c0)/determinant;
    }
    return p;
}

scurve::traject scurve::TrajectCalculator(double Vel, double Acc, double Vo, double Ve, std::vector<waypoint> waypoints){
    traject tr;
    tr.Waypoints = waypoints;
    tr.Ltot = TrajectLenght(tr.Waypoints);
    double D = tr.Ltot;
    double V = Vel;

    double ta = D / (2.0 * V);
    if (ta < 0.001) ta = 0.001;

    double tj = ta / 3.0;
    double a_max = V / (2.0 * tj);
    double j_max = a_max / tj;

    tr.Vel = V;
    tr.Acc_lineair = a_max;
    tr.Jm = j_max;
    tr.Vo = Vo;
    tr.Ve = Ve;

    auto calculate_phases = [&](double target_vel) {
        // --- UP PHASE ---
        double delta_v_up = target_vel - tr.Vo;
        tr.as_up = tr.Acc_lineair;
        if (delta_v_up < (tr.as_up * tr.as_up) / tr.Jm) {
            tr.as_up = sqrt(delta_v_up * tr.Jm);
        }
        tr.t1_up = std::max(0.0, tr.as_up / tr.Jm); // REMOVED std::round for perfect math
        tr.as_up = tr.Jm * tr.t1_up;
        tr.v1_up = tr.Vo + (tr.as_up * tr.t1_up) * 0.5;
        double req_tlin_up = 0.0;
        if (tr.as_up > 0) req_tlin_up = (target_vel - tr.v1_up - (tr.as_up * tr.t1_up) * 0.5) / tr.as_up;
        tr.tlin_up = std::max(0.0, req_tlin_up); // REMOVED std::round
        tr.v2_up = tr.v1_up + tr.as_up * tr.tlin_up;
        double actual_peak_vel = tr.v2_up + (tr.as_up * tr.t1_up) * 0.5;
        double t1_2 = tr.t1_up * tr.t1_up;
        double t1_3 = t1_2 * tr.t1_up;
        double tlin_2 = tr.tlin_up * tr.tlin_up;
        tr.T1 = 2.0 * tr.t1_up + tr.tlin_up;
        tr.L1 = (tr.Vo * tr.t1_up + tr.Jm * t1_3 / 6.0) +
                (tr.v1_up * tr.tlin_up + tr.as_up * tlin_2 * 0.5) +
                (tr.v2_up * tr.t1_up + tr.as_up * t1_2 * 0.5 - tr.Jm * t1_3 / 6.0);

        // --- DOWN PHASE ---
        double delta_v_down = actual_peak_vel - tr.Ve;
        tr.as_down = tr.Acc_lineair;
        if (delta_v_down < (tr.as_down * tr.as_down) / tr.Jm) {
            tr.as_down = sqrt(delta_v_down * tr.Jm);
        }
        tr.t1_down = std::max(0.0, tr.as_down / tr.Jm); // REMOVED std::round
        tr.as_down = tr.Jm * tr.t1_down;
        tr.v2_down = tr.Ve + (tr.as_down * tr.t1_down) * 0.5;
        double req_tlin_down = 0.0;
        if (tr.as_down > 0) req_tlin_down = (actual_peak_vel - tr.v2_down - (tr.as_down * tr.t1_down) * 0.5) / tr.as_down;
        tr.tlin_down = std::max(0.0, req_tlin_down); // REMOVED std::round
        tr.v1_down = tr.v2_down + tr.as_down * tr.tlin_down;
        double td1_2 = tr.t1_down * tr.t1_down;
        double td1_3 = td1_2 * tr.t1_down;
        double tdlin_2 = tr.tlin_down * tr.tlin_down;
        tr.T3 = 2.0 * tr.t1_down + tr.tlin_down;
        tr.L3 = (actual_peak_vel * tr.t1_down - tr.Jm * td1_3 / 6.0) +
                (tr.v1_down * tr.tlin_down - tr.as_down * tdlin_2 * 0.5) +
                (tr.v2_down * tr.t1_down - tr.as_down * td1_2 * 0.5 + tr.Jm * td1_3 / 6.0);
        tr.Vel = actual_peak_vel;
    };

    calculate_phases(tr.Vel);
    tr.L2 = tr.Ltot - tr.L1 - tr.L3;

    if(tr.L2 < 0){
        double v_low = 0.0;
        double v_high = tr.Vel;
        for(int iter = 0; iter < 40; ++iter){
            double v_mid = (v_low + v_high) * 0.5;
            tr.Vel = v_mid;
            calculate_phases(tr.Vel);
            tr.L2 = tr.Ltot - tr.L1 - tr.L3;
            if(tr.L2 < 0){
                v_high = v_mid;
            } else {
                v_low = v_mid;
            }
        }
        tr.Vel = v_low;
        calculate_phases(tr.Vel);
        tr.L2 = 0.0;
        tr.T2 = 0.0;
    } else {
        double required_T2 = tr.L2 / tr.Vel;
        tr.T2 = std::max(0.0, required_T2); // REMOVED std::round
        tr.L2 = tr.T2 * tr.Vel;
    }

    tr.Ttot = tr.T1 + tr.T2 + tr.T3;
    tr.T1h = tr.T1 / 2.0;
    tr.T3h = tr.T3 / 2.0;
    return tr;
}

// Deprecated: Replaced by continuous exact evaluator, leaving these to keep scurve.h satisfied
scurve::traject scurve::ScurveUp(traject tr){ return tr; }
scurve::traject scurve::ScurveSteady(traject tr){ return tr; }
scurve::traject scurve::ScurveDown(traject tr){ return tr; }