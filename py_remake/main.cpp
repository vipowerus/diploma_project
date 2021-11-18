#include <iostream>
#include<cmath>
#include "gurobi_c++.h"
#include "rational.cpp"
using namespace std;

Rational toRatio(double value, int n) { // 1.41429
        Rational l((int)value, 1);
        Rational r((int)value + 1, 1);
        while(l.den + r.den <= n){
            Rational ratio(l.num + r.num, l.den + r.den);
            if(ratio.num == ratio.den * value) return ratio;
            if(ratio.num < ratio.den * value) l = ratio;
            else r = ratio;
        }
        return r;
}

int main() {
    Rational a = toRatio(2.0/3, 10000);
    cout << a.num << " " << a.den << '\n';
    try {
        vector<int> Machines = {1, 2, 3};
        vector<int> Jobs = {1, 2, 3, 4, 5, 6};

        GRBEnv env = GRBEnv(true);
        env.set("LogFile", "result.json");
        env.start();

        // Create an empty model
        GRBModel m = GRBModel(env);

        GRBVar p[Machines.size()][Jobs.size()];
        for (size_t i = 0; i < Machines.size(); i++)
            for (size_t j = 0; j < Jobs.size(); j++)
                p[i][j] = m.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "p" + to_string(i) + to_string(j));
        GRBVar t = m.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "tau");
        GRBVar rho = m.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "rho");

        m.setObjective((GRBLinExpr)rho, GRB_MAXIMIZE);

        // Add bounds
        for (size_t i = 0; i < Machines.size(); i++)
            for (size_t j = 0; j < Jobs.size(); j++)
                m.addConstr(p[i][j] <= 2.0/3, "B" + to_string(i) + to_string(j));
            
        
        // Add constraints: lmax+2tau<=1
        for (size_t i = 0; i < Machines.size(); i++) {
            GRBLinExpr e = 2 * t;
            for (size_t j = 0; j < Jobs.size(); j++)
                e += p[i][j];
            m.addConstr(e <= 1, "L" + to_string(i)); // must be "<"
        }

        // Add constraint: d1<=1
        m.addConstr(p[0][0] + p[1][0] + p[2][0] <= 1,"D0");

        for (size_t j = 1; j < Jobs.size(); j++){
            GRBLinExpr e = 2 * t;
            for (size_t i = 0; i < Machines.size(); i++)
                e += p[i][j];
            m.addConstr(e <= 1, "D" + to_string(j)); // must be "<"
        }

        m.addConstr(rho <= 2 * t + p[0][0] + p[1][0] + p[1][1] + p[1][2] + p[2][2] + p[2][3] + p[2][4], "P1");

        m.update();
        m.optimize();
        m.write("result.lp");

        auto vars = m.getVars();
       for (int i = 0; i < m.get(GRB_IntAttr_NumVars); i++)
            cout << vars[i].get(GRB_StringAttr_VarName) << " " << vars[i].get(GRB_DoubleAttr_X) << '\n';

        auto constrs = m.getConstrs();
        for (int i = 0; i < m.get(GRB_IntAttr_NumConstrs); i++)
            if(constrs[i].get(GRB_IntAttr_CBasis) == -1)
                cout << constrs[i].get(GRB_StringAttr_ConstrName) << " " << constrs[i].get(GRB_DoubleAttr_Pi) << '\n';

    } catch (GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch (...) {
        cout << "Exception during optimization" << endl;
    }

    return 0;
}