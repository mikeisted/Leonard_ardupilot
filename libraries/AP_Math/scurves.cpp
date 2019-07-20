/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <AP_Math/AP_Math.h>
#include "scurves.h"

void scurves::Cal_T(float tin, float J0)
{
    enum jtype_t Jtype = JTYPE_CONSTANT;
    float J = J0;
    float T = oT[num_items] + tin;
    float A = oA[num_items] + J0*tin;
    float V = oV[num_items] + oA[num_items]*tin + 0.5*J0*sq(tin);
    float P = oP[num_items] + oV[num_items]*tin + 0.5*oA[num_items]*sq(tin) + (1/6)*J0*powf(tin,3);
    num_items++;

    oT[num_items] = T;
    oJtype[num_items] = Jtype;
    oJ[num_items] = J;
    oA[num_items] = A;
    oV[num_items] = V;
    oP[num_items] = P;
}

void scurves::Cal_JS1(float tj, float Jp)
{
    float Beta = M_PI/tj;
    float Alpha = Jp/2;
    float AT = Alpha*tj;
    float VT = Alpha*(sq(tj)/2 - 2/sq(Beta));
    float PT = Alpha*((-1/sq(Beta))*tj + (1/6)*powf(tj,3));

    enum jtype_t Jtype = JTYPE_POSITIVE;
    float J = Jp;
    float T = oT[num_items] + tj;
    float A = oA[num_items] + AT;
    float V = oV[num_items] + oA[num_items]*tj + VT;
    float P = oP[num_items] + oV[num_items]*tj + 0.5*oA[num_items]*sq(tj) + PT;
    num_items++;

    oT[num_items] = T;
    oJtype[num_items] = Jtype;
    oJ[num_items] = J;
    oA[num_items] = A;
    oV[num_items] = V;
    oP[num_items] = P;
}

void scurves::Cal_JS2(float tj, float Jp)
{
    float Beta = M_PI/tj;
    float Alpha = Jp/2;
    float AT = Alpha*tj;
    float VT = Alpha*(sq(tj)/2 - 2/sq(Beta));
    float PT = Alpha*((-1/sq(Beta))*tj + (1/6)*powf(tj,3));
    float A2T = Jp*tj;
    float V2T = Jp*sq(tj);
    float P2T = Alpha*((-1/sq(Beta))*2*tj +(4/3)*powf(tj,3));

    enum jtype_t Jtype = JTYPE_NEGATIVE;
    float J = Jp;
    float T = oT[num_items] + tj;
    float A = (oA[num_items]-AT) + A2T;
    float V = (oV[num_items]-VT) + (oA[num_items]-AT)*tj + V2T;
    float P = (oP[num_items]-PT) + (oV[num_items]-VT)*tj + 0.5*(oA[num_items]-AT)*sq(tj) + P2T;
    num_items++;

    oT[num_items] = T;
    oJtype[num_items] = Jtype;
    oJ[num_items] = J;
    oA[num_items] = A;
    oV[num_items] = V;
    oP[num_items] = P;
}

void scurves::Cal_tj_Jp_Tcj(float tj, float Jp, float Tcj)
{
    Cal_JS1(tj, Jp);
    Cal_T(Tcj, Jp);
    Cal_JS2(tj, Jp);
}

void scurves::Cal_Pn(float V0, float P0, float Pp)
{
    float tj = otj;
    float Jp = oJp;
    float Ap = oAp;
    float Vp = oVp;

    float t2, t4, t6;
    Cal_Pos(tj, V0, P0, Jp, Ap, Vp, Pp, Jp, t2, t4, t6);

    Cal_tj_Jp_Tcj(tj, Jp, t2);
    Cal_T(t4, 0.0);
    Cal_tj_Jp_Tcj(tj, -Jp, t6);
    float Tcv = (Pp-oP[num_items])/oV[num_items];
    Cal_T(Tcv, 0.0);

    Cal_Pos(tj, 0, P0, Jp, Ap, Vp, Pp*2, Jp, t2, t4, t6);
    Cal_T(Tcv, 0.0);
    Cal_tj_Jp_Tcj(tj, -Jp, t6);
    Cal_T(t4, 0.0);
    Cal_tj_Jp_Tcj(tj, Jp, t2);
}

void scurves::Cal_Pos(float tj, float V0, float P0, float Jp, float Ap, float Vp, float Pp,
                      float& Jp_out, float& t2_out, float& t4_out, float& t6_out)
{
    Ap = MIN(MIN(Ap, (Vp - V0)/(2*tj)), (Pp - P0 + 4*V0*tj)/(4*sq(tj)));
    if (fabsf(Ap) < Jp*tj) {
        Jp_out = Ap/tj;
        t2_out = 0;
        if ((Vp <= V0 + 2*Ap*tj) || (Pp <= P0 + 4*V0*tj + 4*Ap*sq(tj))) {
            t4_out = 0;
        } else {
            t4_out = MIN(-(V0-Vp+Ap*tj+(Ap*Ap)/Jp)/Ap, \
                    MAX( \
                        ((Ap*Ap)*(-3.0/2.0)+safe_sqrt((Ap*Ap*Ap*Ap)*(1.0/4.0)+(Jp*Jp)*(V0*V0)+(Ap*Ap)*(Jp*Jp)*(tj*tj)*(1.0/4.0)-Ap*(Jp*Jp)*P0*2.0+Ap*(Jp*Jp)*Pp*2.0-(Ap*Ap)*Jp*V0+(Ap*Ap*Ap)*Jp*tj*(1.0/2.0)-Ap*(Jp*Jp)*V0*tj)-Jp*V0-Ap*Jp*tj*(3.0/2.0))/(Ap*Jp), \
                        ((Ap*Ap)*(-3.0/2.0)-safe_sqrt((Ap*Ap*Ap*Ap)*(1.0/4.0)+(Jp*Jp)*(V0*V0)+(Ap*Ap)*(Jp*Jp)*(tj*tj)*(1.0/4.0)-Ap*(Jp*Jp)*P0*2.0+Ap*(Jp*Jp)*Pp*2.0-(Ap*Ap)*Jp*V0+(Ap*Ap*Ap)*Jp*tj*(1.0/2.0)-Ap*(Jp*Jp)*V0*tj)-Jp*V0-Ap*Jp*tj*(3.0/2.0))/(Ap*Jp) \
                        ));
        }
    } else {
        if ((Vp < V0+Ap*tj+(Ap*Ap)/Jp) || (Pp < P0+1.0/(Jp*Jp)*(Ap*Ap*Ap+Ap*Jp*(V0*2.0+Ap*tj*2.0))+V0*tj*2.0+Ap*(tj*tj))) {
            Ap = MIN(MIN(Ap, \
                    MAX( \
                            Jp*(tj+safe_sqrt((V0*-4.0+Vp*4.0+Jp*(tj*tj))/Jp))*(-1.0/2.0), \
                            Jp*(tj-safe_sqrt((V0*-4.0+Vp*4.0+Jp*(tj*tj))/Jp))*(-1.0/2.0))), \
                    Jp*tj*(-2.0/3.0)+((Jp*Jp)*(tj*tj)*(1.0/9.0)-Jp*V0*(2.0/3.0))*1.0/powf(safe_sqrt(powf((Jp*Jp)*P0*(1.0/2.0)-(Jp*Jp)*Pp*(1.0/2.0)+(Jp*Jp*Jp)*(tj*tj*tj)*(8.0/2.7E1)-Jp*tj*((Jp*Jp)*(tj*tj)+Jp*V0*2.0)*(1.0/3.0)+(Jp*Jp)*V0*tj,2.0)-powf((Jp*Jp)*(tj*tj)*(1.0/9.0)-Jp*V0*(2.0/3.0),3.0))-(Jp*Jp)*P0*(1.0/2.0)+(Jp*Jp)*Pp*(1.0/2.0)-(Jp*Jp*Jp)*(tj*tj*tj)*(8.0/2.7E1)+Jp*tj*((Jp*Jp)*(tj*tj)+Jp*V0*2.0)*(1.0/3.0)-(Jp*Jp)*V0*tj,1.0/3.0)+powf(safe_sqrt(powf((Jp*Jp)*P0*(1.0/2.0)-(Jp*Jp)*Pp*(1.0/2.0)+(Jp*Jp*Jp)*(tj*tj*tj)*(8.0/2.7E1)-Jp*tj*((Jp*Jp)*(tj*tj)+Jp*V0*2.0)*(1.0/3.0)+(Jp*Jp)*V0*tj,2.0)-powf((Jp*Jp)*(tj*tj)*(1.0/9.0)-Jp*V0*(2.0/3.0),3.0))-(Jp*Jp)*P0*(1.0/2.0)+(Jp*Jp)*Pp*(1.0/2.0)-(Jp*Jp*Jp)*(tj*tj*tj)*(8.0/2.7E1)+Jp*tj*((Jp*Jp)*(tj*tj)+Jp*V0*2.0)*(1.0/3.0)-(Jp*Jp)*V0*tj,1.0/3.0));
            t4_out = 0;
        } else {
            t4_out = MIN(-(V0-Vp+Ap*tj+(Ap*Ap)/Jp)/Ap, \
                    MAX( \
                            ((Ap*Ap)*(-3.0/2.0)+safe_sqrt((Ap*Ap*Ap*Ap)*(1.0/4.0)+(Jp*Jp)*(V0*V0)+(Ap*Ap)*(Jp*Jp)*(tj*tj)*(1.0/4.0)-Ap*(Jp*Jp)*P0*2.0+Ap*(Jp*Jp)*Pp*2.0-(Ap*Ap)*Jp*V0+(Ap*Ap*Ap)*Jp*tj*(1.0/2.0)-Ap*(Jp*Jp)*V0*tj)-Jp*V0-Ap*Jp*tj*(3.0/2.0))/(Ap*Jp), \
                            ((Ap*Ap)*(-3.0/2.0)-safe_sqrt((Ap*Ap*Ap*Ap)*(1.0/4.0)+(Jp*Jp)*(V0*V0)+(Ap*Ap)*(Jp*Jp)*(tj*tj)*(1.0/4.0)-Ap*(Jp*Jp)*P0*2.0+Ap*(Jp*Jp)*Pp*2.0-(Ap*Ap)*Jp*V0+(Ap*Ap*Ap)*Jp*tj*(1.0/2.0)-Ap*(Jp*Jp)*V0*tj)-Jp*V0-Ap*Jp*tj*(3.0/2.0))/(Ap*Jp)) );
        }
        t2_out = Ap/Jp - tj;
    }
    t6_out = t2_out;
}

void scurves::runme(float t, float& Jt_out, float& At_out, float& Vt_out, float& Pt_out)
{
    jtype_t Jtype;
    int8_t pnt = 0;
    float T, Jp, A0, V0, P0;

    for (int8_t i=0; i<num_items; i++) {
        if (t > oT[i]){
            pnt = i + 1;
        }
    }
    if (pnt == 1) {
        T = oT[pnt];
        Jtype = JTYPE_CONSTANT;
        Jp = 0.0f;
        A0 = oA[pnt];
        V0 = oV[pnt];
        P0 = oP[pnt];
    } else if (pnt == num_items) {
        T = oT[pnt];
        Jtype = JTYPE_CONSTANT;
        Jp = 0.0;
        A0 = oA[pnt];
        V0 = oV[pnt];
        P0 = oP[pnt];
    } else {
        Jtype = oJtype[pnt];
        Jp = oJ[pnt];
        T = oT[pnt-1];
        A0 = oA[pnt-1];
        V0 = oV[pnt-1];
        P0 = oP[pnt-1];
    }

    switch (Jtype) {
        case JTYPE_POSITIVE:
            JSegment1(t-T, Jp, A0, V0, P0, Jt_out, At_out, Vt_out, Pt_out);
            break;
        case JTYPE_NEGATIVE:
            JSegment2(t-T, Jp, A0, V0, P0, Jt_out, At_out, Vt_out, Pt_out);
            break;
        default:
            JConst(t-T, Jp, A0, V0, P0, Jt_out, At_out, Vt_out, Pt_out);
            break;
    }
}

void scurves::JConst(float t, float J0, float A0, float V0, float P0, float& Jt, float& At, float& Vt, float& Pt)
{
    Jt = J0;
    At = A0 + J0*t;
    Vt = V0 + A0*t + 0.5*J0*(t*t);
    Pt = P0 + V0*t + 0.5*A0*(t*t) + (1/6)*J0*(t*t*t);
}

void scurves::JSegment1(float t, float Jp, float A0, float V0, float P0, float& Jt, float& At, float& Vt, float& Pt)
{
    float tj = otj;
    float Alpha = Jp/2;
    float Beta = M_PI/tj;
    Jt = Alpha*(1 - cosf(Beta*t));
    At = A0 + Alpha*t - (Alpha/Beta)*sinf(Beta*t);
    Vt = V0 + A0*t + (Alpha/2)*(t*t) + (Alpha/(Beta*Beta))*cosf(Beta*t) - Alpha/(Beta*Beta);
    Pt = P0 + V0*t + 0.5*A0*(t*t) + (-Alpha/(Beta*Beta))*t + Alpha*(t*t*t)/6 + (Alpha/(Beta*Beta*Beta))*sinf(Beta*t);
}

void scurves::JSegment2(float t, float Jp, float A0, float V0, float P0, float& Jt, float& At, float& Vt, float& Pt)
{
    float tj = otj;
    float Alpha = Jp/2;
    float Beta = M_PI/tj;
    float AT = Alpha*tj;
    float VT = Alpha*((tj*tj)/2 - 2/(Beta*Beta));
    float PT = Alpha*((-1/(Beta*Beta))*tj + (1/6)*(tj*tj*tj));
    Jt = Alpha*(1 - cosf(Beta * (t+tj)));
    At = (A0-AT) + Alpha*(t+tj) - (Alpha / Beta) * sinf(Beta * (t+tj));
    Vt = (V0-VT) + (A0-AT)*t + 0.5*Alpha*(t+tj)*(t+tj) + (Alpha/(Beta*Beta))*cosf(Beta*(t+tj)) - Alpha/(Beta*Beta);
    Pt = (P0-PT) + (V0-VT)*t + 0.5*(A0-AT)*(t*t) + (-Alpha/(Beta*Beta))*(t+tj) + (Alpha/6)*(t+tj)*(t+tj)*(t+tj) + (Alpha/(Beta*Beta*Beta))*sinf(Beta*(t+tj));
}
