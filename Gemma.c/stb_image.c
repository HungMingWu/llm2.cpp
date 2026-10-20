#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RESTRICT restrict
#if defined(__GNUC__) && !defined(__clang__)
# define __GCC__
#endif
// NOLINTBEGIN
// clang-format off

// Suppress diagnostics for the minified stb_image / stb_image_resize2 blob
#if defined(__clang__) || defined(__GCC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdangling-else"
#  pragma GCC diagnostic ignored "-Wmisleading-indentation"
#  pragma GCC diagnostic ignored "-Wunknown-pragmas"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#  pragma GCC diagnostic ignored "-Wunused-variable"
#  pragma GCC diagnostic ignored "-Wsign-compare"
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__MSVC__)
#  pragma warning(push)
#  pragma warning(disable : 4138)  // nonstandard extension used
#  pragma warning(disable : 4244)  // conversion, possible loss of data
#  pragma warning(disable : 4100)  // unreferenced formal parameter
#  pragma warning(disable : 4189)  // local var is inited not referenced
#endif

#define Z1 float////////////////////////////////////////////////////////////////
#define Z2 +0.5f;for(;;){if(G1<0)G1=0;if(G1>////////////////////////////////////
#define Z3 (Z1*Gw,int Gx,void const*Gy){Z1*RESTRICT Gz=Gw;Z1*G0=(Z1*)Gz+Gx;unsi\
gned char const*G1=(unsigned char const*)Gy;////////////////////////////////////
#define Z4 Z3 Gz+=4;while(Gz<=G0){Gz[-4]=///////////////////////////////////////
#define Z5 (Z1*Gw,unsigned int Gx,Z1 const*Gy,B4 const*Gz,Z1 const*G0,int G1){Z\
1 const*G2=Gw+Gx*7;Z1*RESTRICT G3=Gw;do{Z1 const*G4=Gy+Gz->A*7;Z1 const*G5=G0;Z\
1 G6,G7,G8,G9,G_,HA,HB,HC///////////////////////////////////////////////////////
#define Z6 (Z1*Gw,Z1 const*Gx,Z1 const**Gy,Z1 const*Gz){Z1*RESTRICT G0=Gw;Z1 co\
nst*G1=Gy[0];Z1 G2=Gx[0];Z1 const*G3=Gy[1];Z1 G4=Gx[1];Z1 const*G5=Gy[2];Z1 G6=\
Gx[2];Z1 const*G7=Gy[3];Z1 G8=Gx[3];////////////////////////////////////////////
#define Z7 Z5,HD,HE,HF,HG,HH,HI,HJ;HJ=G5[0];G6=G4[0]*HJ;G7=G4[1]*HJ;G8=G4[2]*HJ\
;G9=G4[3]*HJ;G_=G4[4]*HJ;HA=G4[5]*HJ;HB=G4[6]*HJ;HJ=G5[1];HC=G4[7]*HJ;HD=G4[8]*\
HJ;HE=G4[9]*HJ;HF=G4[10]*HJ;HG=G4[11]*HJ;HH=G4[12]*HJ;HI=G4[13]*HJ;HJ=G5[2];G6+\
=G4[14]*HJ;G7+=G4[15]*HJ;G8+=G4[16]*HJ;G9+=G4[17]*HJ;G_+=G4[18]*HJ;HA+=G4[19]*H\
J;HB+=G4[20]*HJ;HJ=G5[3];HC+=G4[21]*HJ;HD+=G4[22]*HJ;HE+=G4[23]*HJ;HF+=G4[24]*H\
J;HG+=G4[25]*HJ;HH+=G4[26]*HJ;HI+=G4[27]*HJ;////////////////////////////////////
#define Z8 Z7 HJ=G5[4];G6+=G4[28]*HJ;G7+=G4[29]*HJ;G8+=G4[30]*HJ;G9+=G4[31]*HJ;\
G_+=G4[32]*HJ;HA+=G4[33]*HJ;HB+=G4[34]*HJ;//////////////////////////////////////
#define Z9 Z8 HJ=G5[5];HC+=G4[35]*HJ;HD+=G4[36]*HJ;HE+=G4[37]*HJ;HF+=G4[38]*HJ;\
HG+=G4[39]*HJ;HH+=G4[40]*HJ;HI+=G4[41]*HJ;//////////////////////////////////////
#define Z0 const*G6=G0;Z1 G7,G8,G9,G_,HA,HB,HC,HD,HE,HF,HG,HH,HI,HJ,HK;HK=G6[0]\
;G7=G4[0]*HK;G8=G4[1]*HK;G9=G4[2]*HK;G_=G4[3]*HK;HA=G4[4]*HK;HB=G4[5]*HK;HC=G4[\
6]*HK;HK=G6[1];HD=G4[7]*HK;HE=G4[8]*HK;HF=G4[9]*HK;HG=G4[10]*HK;HH=G4[11]*HK;HI\
=G4[12]*HK;HJ=G4[13]*HK;HK=G6[2];G7+=G4[14]*HK;G8+=G4[15]*HK;G9+=G4[16]*HK;G_+=\
G4[17]*HK;HA+=G4[18]*HK;HB+=G4[19]*HK;HC+=G4[20]*HK;HK=G6[3];HD+=G4[21]*HK;HE+=\
G4[22]*HK;HF+=G4[23]*HK;HG+=G4[24]*HK;HH+=G4[25]*HK;HI+=G4[26]*HK;HJ+=G4[27]*HK\
;do{G6+=4;G4+=28;HK=G6[0];G7+=G4[0]*HK;G8+=G4[1]*HK;G9+=G4[2]*HK;G_+=G4[3]*HK;H\
A+=G4[4]*HK;HB+=G4[5]*HK;HC+=G4[6]*HK;HK=G6[1];HD+=G4[7]*HK;HE+=G4[8]*HK;HF+=G4\
[9]*HK;HG+=G4[10]*HK;HH+=G4[11]*HK;HI+=G4[12]*HK;HJ+=G4[13]*HK;HK=G6[2];G7+=G4[\
14]*HK;G8+=G4[15]*HK;G9+=G4[16]*HK;G_+=G4[17]*HK;HA+=G4[18]*HK;HB+=G4[19]*HK;HC\
+=G4[20]*HK;HK=G6[3];HD+=G4[21]*HK;HE+=G4[22]*HK;HF+=G4[23]*HK;HG+=G4[24]*HK;HH\
+=G4[25]*HK;HI+=G4[26]*HK;HJ+=G4[27]*HK;--G5;}while(G5>0);//////////////////////
#define Za (Z1*Gw,unsigned int Gx,Z1 const*Gy,B4 const*Gz,Z1 const*G0,int G1){Z\
1 const*G2=Gw+Gx*4;Z1*RESTRICT G3=Gw;do{Z1 const*G4=Gy+Gz->A*4;Z1 const*G5=G0;Z\
1 G6,G7,G8,G9,G_,HA,HB,HC,HD;HD=G5[0];G6=G4[0]*HD;G7=G4[1]*HD;G8=G4[2]*HD;G9=G4\
[3]*HD;HD=G5[1];G_=G4[4]*HD;HA=G4[5]*HD;HB=G4[6]*HD;HC=G4[7]*HD;HD=G5[2];G6+=G4\
[8]*HD;G7+=G4[9]*HD;G8+=G4[10]*HD;G9+=G4[11]*HD;HD=G5[3];G_+=G4[12]*HD;HA+=G4[1\
3]*HD;HB+=G4[14]*HD;HC+=G4[15]*HD;HD=G5[4];G6+=G4[16]*HD;G7+=G4[17]*HD;G8+=G4[1\
8]*HD;G9+=G4[19]*HD;////////////////////////////////////////////////////////////
#define Zb (Z1*Gw,unsigned int Gx,Z1 const*Gy,B4 const*Gz,Z1 const*G0,int G1){Z\
1 const*G2=Gw+Gx*3;Z1*RESTRICT G3=Gw;do{Z1 const*G4=Gy+Gz->A*3;Z1 const*G5=G0;Z\
1 G6,G7,G8,G9,G_,HA,HB,HC,HD,HE,HF,HG,HH;HH=G5[0];G6=G4[0]*HH;G7=G4[1]*HH;G8=G4\
[2]*HH;HH=G5[1];G9=G4[3]*HH;G_=G4[4]*HH;HA=G4[5]*HH;HH=G5[2];HB=G4[6]*HH;HC=G4[\
7]*HH;HD=G4[8]*HH;HH=G5[3];HE=G4[9]*HH;HF=G4[10]*HH;HG=G4[11]*HH;HH=G5[4];G6+=G\
4[12]*HH;G7+=G4[13]*HH;G8+=G4[14]*HH;///////////////////////////////////////////
#define Zc (Z1*Gw,unsigned int Gx,Z1 const*Gy,B4 const*Gz,Z1 const*G0,int G1){Z\
1 const*G2=Gw+Gx*2;Z1*RESTRICT G3=Gw;do{Z1 const*G4=Gy+Gz->A*2;Z1 const*G5=G0;Z\
1 G6,G7,G8,G9,G_,HA,HB,HC,HD;HD=G5[0];G6=G4[0]*HD;G_=G4[1]*HD;HD=G5[1];G7=G4[2]\
*HD;HA=G4[3]*HD;HD=G5[2];G8=G4[4]*HD;HB=G4[5]*HD;HD=G5[3];G9=G4[6]*HD;HC=G4[7]*\
HD;/////////////////////////////////////////////////////////////////////////////
#define Zd (void*Gw,int Gx,Z1 const*Gy){unsigned char*RESTRICT Gz=(unsigned cha\
r*)Gw;unsigned char*G0=((unsigned char*)Gz)+Gx;/////////////////////////////////
#define Ze +3)>>2;Z1 const*G6=G0;Z1 G7,G8,G9,G_,HA,HB,HC,HD,HE;HE=G6[0];G7=G4[0\
]*HE;G8=G4[1]*HE;G9=G4[2]*HE;G_=G4[3]*HE;HE=G6[1];HA=G4[4]*HE;HB=G4[5]*HE;HC=G4\
[6]*HE;HD=G4[7]*HE;HE=G6[2];G7+=G4[8]*HE;G8+=G4[9]*HE;G9+=G4[10]*HE;G_+=G4[11]*\
HE;HE=G6[3];HA+=G4[12]*HE;HB+=G4[13]*HE;HC+=G4[14]*HE;HD+=G4[15]*HE;do{G6+=4;G4\
+=16;HE=G6[0];G7+=G4[0]*HE;G8+=G4[1]*HE;G9+=G4[2]*HE;G_+=G4[3]*HE;HE=G6[1];HA+=\
G4[4]*HE;HB+=G4[5]*HE;HC+=G4[6]*HE;HD+=G4[7]*HE;HE=G6[2];G7+=G4[8]*HE;G8+=G4[9]\
*HE;G9+=G4[10]*HE;G_+=G4[11]*HE;HE=G6[3];HA+=G4[12]*HE;HB+=G4[13]*HE;HC+=G4[14]\
*HE;HD+=G4[15]*HE;--G5;}while(G5>0);////////////////////////////////////////////
#define Zf (Z1*Gw,unsigned int Gx,Z1 const*Gy,B4 const*Gz,Z1 const*G0,int G1){Z\
1 const*G2=Gw+Gx*1;Z1*RESTRICT G3=Gw;do{Z1 const*G4=Gy+Gz->A*1;/////////////////
#define Zg +3)>>2;Z1 const*G6=G0;Z1 G7,G8,G9,G_,HA,HB,HC,HD,HE,HF,HG,HH,HI;HI=G\
6[0];G7=G4[0]*HI;G8=G4[1]*HI;G9=G4[2]*HI;HI=G6[1];G_=G4[3]*HI;HA=G4[4]*HI;HB=G4\
[5]*HI;HI=G6[2];HC=G4[6]*HI;HD=G4[7]*HI;HE=G4[8]*HI;HI=G6[3];HF=G4[9]*HI;HG=G4[\
10]*HI;HH=G4[11]*HI;do{G6+=4;G4+=12;HI=G6[0];G7+=G4[0]*HI;G8+=G4[1]*HI;G9+=G4[2\
]*HI;HI=G6[1];G_+=G4[3]*HI;HA+=G4[4]*HI;HB+=G4[5]*HI;HI=G6[2];HC+=G4[6]*HI;HD+=\
G4[7]*HI;HE+=G4[8]*HI;HI=G6[3];HF+=G4[9]*HI;HG+=G4[10]*HI;HH+=G4[11]*HI;--G5;}w\
hile(G5>0);/////////////////////////////////////////////////////////////////////
#define Zh (Z1**Gw,Z1 const*Gx,Z1 const*Gy,Z1 const*Gz){Z1*RESTRICT G0=Gw[0];;Z\
1 G1=Gx[0];Z1*RESTRICT G2=Gw[1];Z1 G3=Gx[1];Z1*RESTRICT G4=Gw[2];Z1 G5=Gx[2];Z1\
*RESTRICT G6=Gw[3];Z1 G7=Gx[3];/////////////////////////////////////////////////
#define Zi (void*Gw,int Gx,Z1 const*Gy){unsigned short*RESTRICT Gz=(unsigned sh\
ort*)Gw;unsigned short*G0=((unsigned short*)Gz)+Gx;Gz+=4;while(Gz<=G0){Z1 G1;G1\
=Gy[////////////////////////////////////////////////////////////////////////////
#define Zj (Z1*Gw,int Gx,void const*Gy){Z1*RESTRICT Gz=Gw;Z1*G0=(Z1*)Gz+Gx;unsi\
gned short const*G1=(unsigned short const*)Gy;Gz+=4;while(Gz<=G0){Gz[-4]=((Z1)G\
1[//////////////////////////////////////////////////////////////////////////////
#define Zk +3)>>2;Z1 const*G6=G0;Z1 G7,G8,G9,G_,HA,HB,HC,HD,HE;HE=G6[0];G7=G4[0\
]*HE;HA=G4[1]*HE;HE=G6[1];G8=G4[2]*HE;HB=G4[3]*HE;HE=G6[2];G9=G4[4]*HE;HC=G4[5]\
*HE;HE=G6[3];G_=G4[6]*HE;HD=G4[7]*HE;do{G6+=4;G4+=8;HE=G6[0];G7+=G4[0]*HE;HA+=G\
4[1]*HE;HE=G6[1];G8+=G4[2]*HE;HB+=G4[3]*HE;HE=G6[2];G9+=G4[4]*HE;HC+=G4[5]*HE;H\
E=G6[3];G_+=G4[6]*HE;HD+=G4[7]*HE;--G5;}while(G5>0);////////////////////////////
#define Zl Z9 HJ=G5[6];G6+=G4[42]*HJ;G7+=G4[43]*HJ;G8+=G4[44]*HJ;G9+=G4[45]*HJ;\
G_+=G4[46]*HJ;HA+=G4[47]*HJ;HB+=G4[48]*HJ;HJ=G5[7];HC+=G4[49]*HJ;HD+=G4[50]*HJ;\
HE+=G4[51]*HJ;HF+=G4[52]*HJ;HG+=G4[53]*HJ;HH+=G4[54]*HJ;HI+=G4[55]*HJ;HJ=G5[8];\
G6+=G4[56]*HJ;G7+=G4[57]*HJ;G8+=G4[58]*HJ;G9+=G4[59]*HJ;G_+=G4[60]*HJ;HA+=G4[61\
]*HJ;HB+=G4[62]*HJ;/////////////////////////////////////////////////////////////
#define Zm G3[0]=G6+HC;G3[1]=G7+HD;G3[2]=G8+HE;G3[3]=G9+HF;G3[4]=G_+HG;G3[5]=HA\
+HH;G3[6]=HB+HI;G0+=G1;++Gz;G3+=7;}while(G3<G2);}static void////////////////////
#define Zn Za HD=G5[5];G_+=G4[20]*HD;HA+=G4[21]*HD;HB+=G4[22]*HD;HC+=G4[23]*HD;\
HD=G5[6];G6+=G4[24]*HD;G7+=G4[25]*HD;G8+=G4[26]*HD;G9+=G4[27]*HD;HD=G5[7];G_+=G\
4[28]*HD;HA+=G4[29]*HD;HB+=G4[30]*HD;HC+=G4[31]*HD;HD=G5[8];G6+=G4[32]*HD;G7+=G\
4[33]*HD;G8+=G4[34]*HD;G9+=G4[35]*HD;///////////////////////////////////////////
#define Zo (Z1*Gw,unsigned int Gx,Z1 const*Gy,B4 const*Gz,Z1 const*G0,int G1){Z\
1 const*G2=Gw+Gx*3;Z1*RESTRICT G3=Gw;do{Z1 const*G4=Gy+Gz->A*3;/////////////////
#define Zp (Z1*Gw,unsigned int Gx,Z1 const*Gy,B4 const*Gz,Z1 const*G0,int G1){Z\
1 const*G2=Gw+Gx*4;Z1*RESTRICT G3=Gw;do{Z1 const*G4=Gy+Gz->A*4;/////////////////
#define Zq G3[0]=(G6+HB)+(G9+HE);G3[1]=(G7+HC)+(G_+HF);G3[2]=(G8+HD)+(HA+HG);G0\
+=G1;++Gz;G3+=3;}while(G3<G2);}static void//////////////////////////////////////
#define Zr G1[3]*G2;HG+=G3[0]*G4;HH+=G3[1]*G4;HI+=G3[2]*G4;HJ+=G3[3]*G4;HG+=G5[\
0]*G6;HH+=G5[1]*G6;HI+=G5[2]*G6;HJ+=G5[3]*G6;HG+=G7[0]*G8;HH+=G7[1]*G8;HI+=G7[2\
]*G8;HJ+=G7[3]*G8;HG+=G9[0]*G_;HH+=G9[1]*G_;HI+=G9[2]*G_;HJ+=G9[3]*G_;HG+=HA[0]\
*HB;HH+=HA[1]*HB;HI+=HA[2]*HB;HJ+=HA[3]*HB;HG+=HC[0]*HD;HH+=HC[1]*HD;HI+=HC[2]*\
HD;HJ+=HC[3]*HD;HG+=HE[0]*HF;HH+=HE[1]*HF;HI+=HE[2]*HF;HJ+=HE[3]*HF;G0[0]=HG;G0\
[1]=HH;G0[2]=HI;G0[3]=HJ;G0+=4;G1+=4;G3+=4;G5+=4;G7+=4;G9+=4;HA+=4;HC+=4;HE+=4;\
}while(G1<Gz){Z1 HG;HG=/////////////////////////////////////////////////////////
#define Zs (Z1*Gw,unsigned int Gx,Z1 const*Gy,B4 const*Gz,Z1 const*G0,int G1){Z\
1 const*G2=Gw+Gx*2;Z1*RESTRICT G3=Gw;do{Z1 const*G4=Gy+Gz->A*2;/////////////////
#define Zt Zf Z1 const*G5=G0;Z1 G6,G7,G8,G9;G6=G4[0]*G5[0];G7=G4[1]*G5[1];G8=G4\
[2]*G5[2];G9=G4[3]*G5[3];G6+=G4[4]*G5[4];G7+=G4[5]*G5[5];///////////////////////
#define Zu G1[3]*G2;HE+=G3[0]*G4;HF+=G3[1]*G4;HG+=G3[2]*G4;HH+=G3[3]*G4;HE+=G5[\
0]*G6;HF+=G5[1]*G6;HG+=G5[2]*G6;HH+=G5[3]*G6;HE+=G7[0]*G8;HF+=G7[1]*G8;HG+=G7[2\
]*G8;HH+=G7[3]*G8;HE+=G9[0]*G_;HF+=G9[1]*G_;HG+=G9[2]*G_;HH+=G9[3]*G_;HE+=HA[0]\
*HB;HF+=HA[1]*HB;HG+=HA[2]*HB;HH+=HA[3]*HB;HE+=HC[0]*HD;HF+=HC[1]*HD;HG+=HC[2]*\
HD;HH+=HC[3]*HD;G0[0]=HE;G0[1]=HF;G0[2]=HG;G0[3]=HH;G0+=4;G1+=4;G3+=4;G5+=4;G7+\
=4;G9+=4;HA+=4;HC+=4;}while(G1<Gz){Z1 HE;HE=////////////////////////////////////
#define Zv Zb HH=G5[5];G9+=G4[15]*HH;G_+=G4[16]*HH;HA+=G4[17]*HH;HH=G5[6];HB+=G\
4[18]*HH;HC+=G4[19]*HH;HD+=G4[20]*HH;HH=G5[7];HE+=G4[21]*HH;HF+=G4[22]*HH;HG+=G\
4[23]*HH;HH=G5[8];G6+=G4[24]*HH;G7+=G4[25]*HH;G8+=G4[26]*HH;////////////////////
#define Zw +3)>>2;Z1 const*G6=G0;Z1 G7,G8,G9,G_;G7=G4[0]*G6[0];G8=G4[1]*G6[1];G\
9=G4[2]*G6[2];G_=G4[3]*G6[3];do{G6+=4;G4+=4;G7+=G4[0]*G6[0];G8+=G4[1]*G6[1];G9+\
=G4[2]*G6[2];G_+=G4[3]*G6[3];--G5;}while(G5>0);/////////////////////////////////
#define Zx Zc HD=G5[4];G6+=G4[8]*HD;G_+=G4[9]*HD;HD=G5[5];G7+=G4[10]*HD;HA+=G4[\
11]*HD;HD=G5[6];G8+=G4[12]*HD;HB+=G4[13]*HD;HD=G5[7];G9+=G4[14]*HD;HC+=G4[15]*H\
D;//////////////////////////////////////////////////////////////////////////////
#define Zy G1[3]*G2;HC+=G3[0]*G4;HD+=G3[1]*G4;HE+=G3[2]*G4;HF+=G3[3]*G4;HC+=G5[\
0]*G6;HD+=G5[1]*G6;HE+=G5[2]*G6;HF+=G5[3]*G6;HC+=G7[0]*G8;HD+=G7[1]*G8;HE+=G7[2\
]*G8;HF+=G7[3]*G8;HC+=G9[0]*G_;HD+=G9[1]*G_;HE+=G9[2]*G_;HF+=G9[3]*G_;HC+=HA[0]\
*HB;HD+=HA[1]*HB;HE+=HA[2]*HB;HF+=HA[3]*HB;G0[0]=HC;G0[1]=HD;G0[2]=HE;G0[3]=HF;\
G0+=4;G1+=4;G3+=4;G5+=4;G7+=4;G9+=4;HA+=4;}while(G1<Gz){Z1 HC;HC=///////////////
////////////////////////////////////////////////////////////////////////////////
enum{STBI_default=0,STBI_grey=1,STBI_grey_alpha=2,STBI_rgb=3,STBI_rgb_alpha=4};
typedef unsigned char A;typedef unsigned short B;typedef struct{int(*A)(void*,//
char*,int);void(*B)(void*,int);int(*C)(void*);}C;typedef uint16_t D;typedef/////
int16_t E;typedef uint32_t F;typedef int32_t G;typedef unsigned char H[1];//////
typedef struct{F A,B;int C,D;C E;void*F;int G;int H;A I[128];int J;A*K,*L;A*M,*N
;}I;static void J(I*Gw);static void K(I*Gw,A const*Gx,int Gy){Gw->E.A=0;Gw->G=0;
Gw->J=0;Gw->K=Gw->M=(A*)Gx;Gw->L=Gw->N=(A*)Gx+Gy;}static void L(I*Gw,C*Gx,void*
Gy){Gw->E=*Gx;Gw->F=Gy;Gw->H=128;Gw->G=1;Gw->J=0;Gw->K=Gw->M=Gw->I;J(Gw);Gw->N=
Gw->L;}static int M(void*Gw,char*Gx,int Gy){return(int)fread(Gx,1,Gy,(FILE*)Gw);
}static void N(void*Gw,int Gx){int Gy;fseek((FILE*)Gw,Gx,1);Gy=fgetc((FILE*)Gw);
if(Gy!=1)ungetc(Gy,(FILE*)Gw);}static int O(void*Gw){return feof((FILE*)Gw)||///
ferror((FILE*)Gw);}static C P={M,N,O,};static void Q(I*Gw,FILE*Gx){L(Gw,&P,(void
*)Gx);}static void R(I*Gw){Gw->K=Gw->M;Gw->L=Gw->N;}enum{STBI_ORDER_RGB,////////
STBI_ORDER_BGR};typedef struct{int A;int B;int C;}S;static int T(I*Gw);static///
void*U(I*Gw,int*Gx,int*Gy,int*Gz,int G0,S*G1);static int V(I*Gw,int*Gx,int*Gy,//
int*Gz);static int W(I*Gw);static void*X(I*Gw,int*Gx,int*Gy,int*Gz,int G0,S*G1);
static int Y(I*Gw,int*Gx,int*Gy,int*Gz);static int Z(I*Gw);static _Thread_local
const char*a;extern const char*stbi_failure_reason(void){return a;}static int b(
const char*Gw){a=Gw;return 0;}static void*c(size_t Gw){return malloc(Gw);}static
int d(int Gw,int Gx){if(Gx<0)return 0;return Gw<=INT_MAX-Gx;}static int e(int Gw
,int Gx){if(Gw<0||Gx<0)return 0;if(Gx==0)return 1;return Gw<=INT_MAX/Gx;}static
int f(int Gw,int Gx,int Gy){return e(Gw,Gx)&&d(Gw*Gx,Gy);}static int g(int Gw,//
int Gx,int Gy,int Gz){return e(Gw,Gx)&&e(Gw*Gx,Gy)&&d(Gw*Gx*Gy,Gz);}static int h
(int Gw,int Gx,int Gy,int Gz,int G0){return e(Gw,Gx)&&e(Gw*Gx,Gy)&&e(Gw*Gx*Gy,Gz
)&&d(Gw*Gx*Gy*Gz,G0);}static void*i(int Gw,int Gx,int Gy){if(!f(Gw,Gx,Gy))return
0;return c(Gw*Gx+Gy);}static void*j(int Gw,int Gx,int Gy,int Gz){if(!g(Gw,Gx,Gy,
Gz))return 0;return c(Gw*Gx*Gy+Gz);}static void*k(int Gw,int Gx,int Gy,int Gz,//
int G0){if(!h(Gw,Gx,Gy,Gz,G0))return 0;return c(Gw*Gx*Gy*Gz+G0);}static int l(//
int a,int b){if((a>=0)!=(b>=0))return 1;if(a<0&&b<0)return a>=INT_MIN-b;return a
<=INT_MAX-b;}static int m(int a,int b){if(b==0||b==-1)return 1;if((a>=0)==(b>=0)
)return a<=32767/b;if(a<0)return a>=-32768/b;return a>=32768/b;}extern void/////
stbi_image_free(void*Gw){free(Gw);}static Z1*n(A*Gw,int Gx,int Gy,int Gz);static
int o=0;extern void stbi_set_flip_vertically_on_load(int Gw){o=Gw;}static///////
_Thread_local int p,q;extern void stbi_set_flip_vertically_on_load_thread(int Gw
){p=Gw;q=1;}static void*r(I*Gw,int*Gx,int*Gy,int*Gz,int G0,S*G1,int G2){memset(
G1,0,12);G1->A=8;G1->C=STBI_ORDER_RGB;G1->B=0;if(W(Gw))return X(Gw,Gx,Gy,Gz,G0,
G1);if(T(Gw))return U(Gw,Gx,Gy,Gz,G0,G1);return((unsigned char*)(size_t)(b(/////
"unknown image type")?0:0));}static A*s(D*Gw,int Gx,int Gy,int Gz){int G0;int G1
=Gx*Gy*Gz;A*G2;G2=(A*)c(G1);if(G2==0)return(unsigned char*)(size_t)(b("outofmem"
)?0:0);for(G0=0;G0<G1;++G0)G2[G0]=(A)((Gw[G0]>>8)&0xFF);free(Gw);return G2;}////
static D*t(A*Gw,int Gx,int Gy,int Gz){int G0;int G1=Gx*Gy*Gz;D*G2;G2=(D*)c(G1*2)
;if(G2==0)return(D*)((unsigned char*)(size_t)(b("outofmem")?0:0));for(G0=0;G0<G1
;++G0)G2[G0]=(D)((Gw[G0]<<8)+Gw[G0]);free(Gw);return G2;}static void u(void*Gw,
int Gx,int Gy,int Gz){int G0;size_t G1=(size_t)Gx*Gz;A G2[2048];A*G3=(A*)Gw;for(
G0=0;G0<(Gy>>1);G0++){A*G4=G3+G0*G1;A*G5=G3+(Gy-G0-1)*G1;size_t G6=G1;while(G6){
size_t G7=(G6<2048)?G6:2048;memcpy(G2,G4,G7);memcpy(G4,G5,G7);memcpy(G5,G2,G7);
G4+=G7;G5+=G7;G6-=G7;}}}static unsigned char*v(I*Gw,int*Gx,int*Gy,int*Gz,int G0)
{S G1;void*G2=r(Gw,Gx,Gy,Gz,G0,&G1,8);if(G2==0)return 0;if(G1.A!=8){G2=s((D*)G2,
*Gx,*Gy,G0==0?*Gz:G0);G1.A=8;}if(q?p:o){int G3=G0?G0:*Gz;u(G2,*Gx,*Gy,G3*1);}///
return(unsigned char*)G2;}static D*w(I*Gw,int*Gx,int*Gy,int*Gz,int G0){S G1;void
*G2=r(Gw,Gx,Gy,Gz,G0,&G1,16);if(G2==0)return 0;if(G1.A!=16){G2=t((A*)G2,*Gx,*Gy,
G0==0?*Gz:G0);G1.A=16;}if(q?p:o){int G3=G0?G0:*Gz;u(G2,*Gx,*Gy,G3*2);}return(D*)
G2;}static FILE*x(char const*Gw,char const*Gx){FILE*Gy;Gy=fopen(Gw,Gx);return Gy
;}extern A*stbi_load_from_file(FILE*Gw,int*Gx,int*Gy,int*Gz,int G0){unsigned////
char*G1;I G2;Q(&G2,Gw);G1=v(&G2,Gx,Gy,Gz,G0);if(G1)fseek(Gw,-(int)(G2.L-G2.K),1)
;return G1;}extern A*stbi_load(char const*Gw,int*Gx,int*Gy,int*Gz,int G0){FILE*
G1=x(Gw,"rb");unsigned char*G2;if(!G1)return(unsigned char*)(size_t)(b("can't f"
"open")?0:0);G2=stbi_load_from_file(G1,Gx,Gy,Gz,G0);fclose(G1);return G2;}extern
D*stbi_load_from_file_16(FILE*Gw,int*Gx,int*Gy,int*Gz,int G0){D*G1;I G2;Q(&G2,Gw
);G1=w(&G2,Gx,Gy,Gz,G0);if(G1)fseek(Gw,-(int)(G2.L-G2.K),1);return G1;}extern B*
stbi_load_16(char const*Gw,int*Gx,int*Gy,int*Gz,int G0){FILE*G1=x(Gw,"rb");D*G2;
if(!G1)return(B*)((unsigned char*)(size_t)(b("can't fopen")?0:0));G2=///////////
stbi_load_from_file_16(G1,Gx,Gy,Gz,G0);fclose(G1);return G2;}extern B*//////////
stbi_load_16_from_memory(A const*Gw,int Gx,int*Gy,int*Gz,int*G0,int G1){I G2;K(&
G2,Gw,Gx);return w(&G2,Gy,Gz,G0,G1);}extern B*stbi_load_16_from_callbacks(C/////
const*Gw,void*Gx,int*Gy,int*Gz,int*G0,int G1){I G2;L(&G2,(C*)Gw,Gx);return w(&G2
,Gy,Gz,G0,G1);}extern A*stbi_load_from_memory(A const*Gw,int Gx,int*Gy,int*Gz,//
int*G0,int G1){I G2;K(&G2,Gw,Gx);return v(&G2,Gy,Gz,G0,G1);}extern A*///////////
stbi_load_from_callbacks(C const*Gw,void*Gx,int*Gy,int*Gz,int*G0,int G1){I G2;L(
&G2,(C*)Gw,Gx);return v(&G2,Gy,Gz,G0,G1);}static Z1*y(I*Gw,int*Gx,int*Gy,int*Gz,
int G0){unsigned char*G1;G1=v(Gw,Gx,Gy,Gz,G0);if(G1)return n(G1,*Gx,*Gy,G0?G0:*
Gz);return(Z1*)(size_t)(b("unknown image type")?0:0);}extern Z1*////////////////
stbi_loadf_from_memory(A const*Gw,int Gx,int*Gy,int*Gz,int*G0,int G1){I G2;K(&G2
,Gw,Gx);return y(&G2,Gy,Gz,G0,G1);}extern Z1*stbi_loadf_from_callbacks(C const*
Gw,void*Gx,int*Gy,int*Gz,int*G0,int G1){I G2;L(&G2,(C*)Gw,Gx);return y(&G2,Gy,Gz
,G0,G1);}extern Z1*stbi_loadf_from_file(FILE*Gw,int*Gx,int*Gy,int*Gz,int G0){I//
G1;Q(&G1,Gw);return y(&G1,Gx,Gy,Gz,G0);}extern Z1*stbi_loadf(char const*Gw,int*
Gx,int*Gy,int*Gz,int G0){Z1*G1;FILE*G2=x(Gw,"rb");if(!G2)return(Z1*)(size_t)(b(
"can't fopen")?0:0);G1=stbi_loadf_from_file(G2,Gx,Gy,Gz,G0);fclose(G2);return G1
;}extern int stbi_is_hdr_from_memory(A const*Gw,int Gx){return 0;}extern int////
stbi_is_hdr_from_file(FILE*Gw){(void)Gw;return 0;}extern int stbi_is_hdr(char///
const*Gw){FILE*Gx=x(Gw,"rb");int Gy=0;if(Gx){Gy=stbi_is_hdr_from_file(Gx);fclose
(Gx);}return Gy;}extern int stbi_is_hdr_from_callbacks(C const*Gw,void*Gx){(void
)Gw;(void)Gx;return 0;}static Z1 z=2.2f,_=1.0f;extern void stbi_ldr_to_hdr_gamma
(Z1 Gw){z=Gw;}extern void stbi_ldr_to_hdr_scale(Z1 Gw){_=Gw;}static Z1 AA=//////
4.545e-01,AB=1.0f;extern void stbi_hdr_to_ldr_gamma(Z1 Gw){AA=1/Gw;}extern void
stbi_hdr_to_ldr_scale(Z1 Gw){AB=1/Gw;}enum{STBI__SCAN_load=0,STBI__SCAN_type,///
STBI__SCAN_header};static void J(I*Gx){int Gy=Gx->E.A(Gx->F,(char*)Gx->I,Gx->H);
Gx->J+=(int)(Gx->K-Gx->M);if(Gy==0){Gx->G=0;Gx->K=Gx->I;Gx->L=Gx->I+1;*Gx->K=0;}
else{Gx->K=Gx->I;Gx->L=Gx->I+Gy;}}static A AC(I*Gw){if(Gw->K<Gw->L)return*Gw->K
++;if(Gw->G){J(Gw);return*Gw->K++;}return 0;}static int AD(I*Gw){if(Gw->E.A){if(
!Gw->E.C(Gw->F))return 0;if(Gw->G==0)return 1;}return Gw->K>=Gw->L;}static void
AE(I*Gw,int Gx){if(Gx==0)return;if(Gx<0){Gw->K=Gw->L;return;}if(Gw->E.A){int Gy=
(int)(Gw->L-Gw->K);if(Gy<Gx){Gw->K=Gw->L;Gw->E.B(Gw->F,Gx-Gy);return;}}Gw->K+=Gx
;}static int AF(I*Gw,A*Gx,int Gy){if(Gw->E.A){int Gz=(int)(Gw->L-Gw->K);if(Gz<Gy
){int G0,G1;memcpy(Gx,Gw->K,Gz);G1=Gw->E.A(Gw->F,(char*)Gx+Gz,Gy-Gz);G0=(G1==(Gy
-Gz));Gw->K=Gw->L;return G0;}}if(Gw->K+Gy<=Gw->L){memcpy(Gx,Gw->K,Gy);Gw->K+=Gy;
return 1;}else return 0;}static int AG(I*Gw){int Gx=AC(Gw);return(Gx<<8)+AC(Gw);
}static F AH(I*Gw){F Gx=AG(Gw);return(Gx<<16)+AG(Gw);}static A AI(int Gw,int Gx,
int Gy){return(A)(((Gw*77)+(Gx*150)+(29*Gy))>>8);}static unsigned char*AJ(//////
unsigned char*Gw,int Gx,int Gy,unsigned int Gz,unsigned int G0){int G1,G2;//////
unsigned char*G3;if(Gy==Gx)return Gw;G3=(unsigned char*)j(Gy,Gz,G0,0);if(G3==0){
free(Gw);return(unsigned char*)(size_t)(b("outofmem")?0:0);}for(G2=0;G2<(int)G0;
++G2){unsigned char*G4=Gw+G2*Gz*Gx;unsigned char*G5=G3+G2*Gz*Gy;switch((Gx*8+Gy)
){case 10:for(G1=Gz-1;G1>=0;--G1,G4+=1,G5+=2){G5[0]=G4[0];G5[1]=255;}break;case
11:for(G1=Gz-1;G1>=0;--G1,G4+=1,G5+=3)G5[0]=G5[1]=G5[2]=G4[0];break;case 12:for(
G1=Gz-1;G1>=0;--G1,G4+=1,G5+=4){G5[0]=G5[1]=G5[2]=G4[0];G5[3]=255;}break;case 17
:for(G1=Gz-1;G1>=0;--G1,G4+=2,G5+=1)G5[0]=G4[0];break;case 19:for(G1=Gz-1;G1>=0;
--G1,G4+=2,G5+=3)G5[0]=G5[1]=G5[2]=G4[0];break;case 20:for(G1=Gz-1;G1>=0;--G1,G4
+=2,G5+=4){G5[0]=G5[1]=G5[2]=G4[0];G5[3]=G4[1];}break;case 28:for(G1=Gz-1;G1>=0;
--G1,G4+=3,G5+=4){G5[0]=G4[0];G5[1]=G4[1];G5[2]=G4[2];G5[3]=255;}break;case 25:
for(G1=Gz-1;G1>=0;--G1,G4+=3,G5+=1)G5[0]=AI(G4[0],G4[1],G4[2]);break;case 26:for
(G1=Gz-1;G1>=0;--G1,G4+=3,G5+=2){G5[0]=AI(G4[0],G4[1],G4[2]);G5[1]=255;}break;//
case 33:for(G1=Gz-1;G1>=0;--G1,G4+=4,G5+=1)G5[0]=AI(G4[0],G4[1],G4[2]);break;///
case 34:for(G1=Gz-1;G1>=0;--G1,G4+=4,G5+=2){G5[0]=AI(G4[0],G4[1],G4[2]);G5[1]=G4
[3];}break;case 35:for(G1=Gz-1;G1>=0;--G1,G4+=4,G5+=3){G5[0]=G4[0];G5[1]=G4[1];
G5[2]=G4[2];}break;default:free(Gw);free(G3);return(unsigned char*)(size_t)(b(//
"unsupported")?0:0);}}free(Gw);return G3;}static D AK(int Gw,int Gx,int Gy){////
return(D)(((Gw*77)+(Gx*150)+(29*Gy))>>8);}static D*AL(D*Gw,int Gx,int Gy,///////
unsigned int Gz,unsigned int G0){int G1,G2;D*G3;if(Gy==Gx)return Gw;G3=(D*)c(Gy*
Gz*G0*2);if(G3==0){free(Gw);return(D*)((unsigned char*)(size_t)(b("outofmem")?0:
0));}for(G2=0;G2<(int)G0;++G2){D*G4=Gw+G2*Gz*Gx;D*G5=G3+G2*Gz*Gy;switch((Gx*8+Gy
)){case 10:for(G1=Gz-1;G1>=0;--G1,G4+=1,G5+=2){G5[0]=G4[0];G5[1]=0xffff;}break;
case 11:for(G1=Gz-1;G1>=0;--G1,G4+=1,G5+=3)G5[0]=G5[1]=G5[2]=G4[0];break;case 12
:for(G1=Gz-1;G1>=0;--G1,G4+=1,G5+=4){G5[0]=G5[1]=G5[2]=G4[0];G5[3]=0xffff;}break
;case 17:for(G1=Gz-1;G1>=0;--G1,G4+=2,G5+=1)G5[0]=G4[0];break;case 19:for(G1=Gz-
1;G1>=0;--G1,G4+=2,G5+=3)G5[0]=G5[1]=G5[2]=G4[0];break;case 20:for(G1=Gz-1;G1>=0
;--G1,G4+=2,G5+=4){G5[0]=G5[1]=G5[2]=G4[0];G5[3]=G4[1];}break;case 28:for(G1=Gz-
1;G1>=0;--G1,G4+=3,G5+=4){G5[0]=G4[0];G5[1]=G4[1];G5[2]=G4[2];G5[3]=0xffff;}////
break;case 25:for(G1=Gz-1;G1>=0;--G1,G4+=3,G5+=1)G5[0]=AK(G4[0],G4[1],G4[2]);///
break;case 26:for(G1=Gz-1;G1>=0;--G1,G4+=3,G5+=2){G5[0]=AK(G4[0],G4[1],G4[2]);G5
[1]=0xffff;}break;case 33:for(G1=Gz-1;G1>=0;--G1,G4+=4,G5+=1)G5[0]=AK(G4[0],G4[1
],G4[2]);break;case 34:for(G1=Gz-1;G1>=0;--G1,G4+=4,G5+=2){G5[0]=AK(G4[0],G4[1],
G4[2]);G5[1]=G4[3];}break;case 35:for(G1=Gz-1;G1>=0;--G1,G4+=4,G5+=3){G5[0]=G4[0
];G5[1]=G4[1];G5[2]=G4[2];}break;default:free(Gw);free(G3);return(D*)((unsigned
char*)(size_t)(b("unsupported")?0:0));}}free(Gw);return G3;}static Z1*n(A*G0,int
G1,int G2,int G3){int G4,G5,G6;Z1*G7;if(!G0)return 0;G7=(Z1*)k(G1,G2,G3,4,0);if(
G7==0){free(G0);return(Z1*)(size_t)(b("outofmem")?0:0);}if(G3&1)G6=G3;else G6=G3
-1;for(G4=0;G4<G1*G2;++G4)for(G5=0;G5<G6;++G5)G7[G4*G3+G5]=(Z1)(pow(G0[G4*G3+G5]
/255.0f,z)*_);if(G6<G3)for(G4=0;G4<G1*G2;++G4)G7[G4*G3+G6]=G0[G4*G3+G6]/255.0f;
free(G0);return G7;}typedef struct{A A[512];D B[256];A C[256];A D[257];unsigned
int E[18];int F[17];}AM;typedef struct{I*A;AM B[4];AM C[4];D D[4][64];E E[4][512
];int F,G;int H,I;int J,K;struct{int A;int B,C;int D;int E,F;int G;int H,I,J,K;A
*L;void*M,*N;A*O;short*P;int Q,R;}L[4];F M;int N;unsigned char O;int P;int Q;int
R;int S;int T;int U;int V;int W;int X;int Y;int Z,a[4];int b,c;void(*d)(A*,int,
short[64]);void(*e)(A*,const A*,const A*,const A*,int,int);A*(*f)(A*,A*,A*,int,
int);}AN;static int AO(AM*Gw,int*Gx){int Gy,Gz,G0=0;unsigned int G1;for(Gy=0;Gy<
16;++Gy)for(Gz=0;Gz<Gx[Gy];++Gz){Gw->D[G0++]=(A)(Gy+1);if(G0>=257)return b(/////
"bad size list");}Gw->D[G0]=0;G1=0;G0=0;for(Gz=1;Gz<=16;++Gz){Gw->F[Gz]=G0-G1;if
(Gw->D[G0]==Gz){while(Gw->D[G0]==Gz)Gw->B[G0++]=(D)(G1++);if(G1-1>=(1u<<Gz))////
return b("bad code lengths");}Gw->E[Gz]=G1<<(16-Gz);G1<<=1;}Gw->E[Gz]=0xffffffff
;memset(Gw->A,255,512);for(Gy=0;Gy<G0;++Gy){int G2=Gw->D[Gy];if(G2<=9){int G3=Gw
->B[Gy]<<(9-G2);int G4=1<<(9-G2);for(Gz=0;Gz<G4;++Gz)Gw->A[G3+Gz]=(A)Gy;}}return
1;}static void AP(E*Gw,AM*Gx){int Gy;for(Gy=0;Gy<512;++Gy){A Gz=Gx->A[Gy];Gw[Gy]
=0;if(Gz<255){int G0=Gx->C[Gz];int G1=(G0>>4)&15;int G2=G0&15;int G3=Gx->D[Gz];
if(G2&&G3+G2<=9){int G4=((Gy<<G3)&511)>>(9-G2);int G5=1<<(G2-1);if(G4<G5)G4+=(~
0U<<G2)+1;if(G4>=-128&&G4<=127)Gw[Gy]=(E)((G4*256)+(G1*16)+(G3+G2));}}}}static//
void AQ(AN*Gw){do{unsigned int Gx=Gw->P?0:AC(Gw->A);if(Gx==0xff){int Gy=AC(Gw->A
);while(Gy==0xff)Gy=AC(Gw->A);if(Gy!=0){Gw->O=(unsigned char)Gy;Gw->P=1;return;}
}Gw->M|=Gx<<(24-Gw->N);Gw->N+=8;}while(Gw->N<=24);}static const F AR[17]={0,1,3,
7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767,65535};static int AS(AN*
Gw,AM*Gx){unsigned int Gy;int Gz,G0;if(Gw->N<16)AQ(Gw);Gz=(Gw->M>>23)&511;G0=Gx
->A[Gz];if(G0<255){int G1=Gx->D[G0];if(G1>Gw->N)return-1;Gw->M<<=G1;Gw->N-=G1;//
return Gx->C[G0];}Gy=Gw->M>>16;for(G0=10;;++G0)if(Gy<Gx->E[G0])break;if(G0==17){
Gw->N-=16;return-1;}if(G0>Gw->N)return-1;Gz=((Gw->M>>(32-G0))&AR[G0])+Gx->F[G0];
if(Gz<0||Gz>=256)return-1;Gw->N-=G0;Gw->M<<=G0;return Gx->C[Gz];}static const///
int AT[16]={0,-1,-3,-7,-15,-31,-63,-127,-255,-511,-1023,-2047,-4095,-8191,-16383
,-32767};static int AU(AN*Gw,int Gx){unsigned int Gy;int Gz;if(Gw->N<Gx)AQ(Gw);
if(Gw->N<Gx)return 0;Gz=Gw->M>>31;Gy=((Gw->M<<Gx)|(Gw->M>>(-Gx&31)));Gw->M=Gy&~
AR[Gx];Gy&=AR[Gx];Gw->N-=Gx;return Gy+(AT[Gx]&(Gz-1));}static int AV(AN*Gw,int//
Gx){unsigned int Gy;if(Gw->N<Gx)AQ(Gw);if(Gw->N<Gx)return 0;Gy=((Gw->M<<Gx)|(Gw
->M>>(-Gx&31)));Gw->M=Gy&~AR[Gx];Gy&=AR[Gx];Gw->N-=Gx;return Gy;}static int AW(
AN*Gw){unsigned int Gx;if(Gw->N<1)AQ(Gw);if(Gw->N<1)return 0;Gx=Gw->M;Gw->M<<=1;
--Gw->N;return Gx&0x80000000;}static const A AX[79]={0,1,8,16,9,2,3,10,17,24,32,
25,18,11,4,5,12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,35,42,49,56,57,50,43,
36,29,22,15,23,30,37,44,51,58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63,63,63
,63,63,63,63,63,63,63,63,63,63,63,63,63};static int AY(AN*Gw,short Gx[64],AM*Gy,
AM*Gz,E*G0,int G1,D*G2){int G3,G4,G5;int G6;if(Gw->N<16)AQ(Gw);G6=AS(Gw,Gy);if(
G6<0||G6>15)return b("bad huffman code");memset(Gx,0,128);G3=G6?AU(Gw,G6):0;if(!
l(Gw->L[G1].G,G3))return b("bad delta");G4=Gw->L[G1].G+G3;Gw->L[G1].G=G4;if(!m(
G4,G2[0]))return b("can't merge dc and ac");Gx[0]=(short)(G4*G2[0]);G5=1;do{////
unsigned int G7;int G8,G9,G_;if(Gw->N<16)AQ(Gw);G8=(Gw->M>>23)&511;G9=G0[G8];if(
G9){G5+=(G9>>4)&15;G_=G9&15;if(G_>Gw->N)return b("bad huffman code");Gw->M<<=G_;
Gw->N-=G_;G7=AX[G5++];Gx[G7]=(short)((G9>>8)*G2[G7]);}else{int HA=AS(Gw,Gz);if(
HA<0)return b("bad huffman code");G_=HA&15;G9=HA>>4;if(G_==0){if(HA!=0xf0)break;
G5+=16;}else{G5+=G9;G7=AX[G5++];Gx[G7]=(short)(AU(Gw,G_)*G2[G7]);}}}while(G5<64)
;return 1;}static int AZ(AN*Gw,short Gx[64],AM*Gy,int Gz){int G0,G1;int G2;if(Gw
->S!=0)return b("can't merge dc and ac");if(Gw->N<16)AQ(Gw);if(Gw->T==0){memset(
Gx,0,128);G2=AS(Gw,Gy);if(G2<0||G2>15)return b("can't merge dc and ac");G0=G2?AU
(Gw,G2):0;if(!l(Gw->L[Gz].G,G0))return b("bad delta");G1=Gw->L[Gz].G+G0;Gw->L[Gz
].G=G1;if(!m(G1,1<<Gw->U))return b("can't merge dc and ac");Gx[0]=(short)(G1*(1
<<Gw->U));}else if(AW(Gw))Gx[0]+=(short)(1<<Gw->U);return 1;}static int Aa(AN*Gw
,short Gx[64],AM*Gy,E*Gz){int G0;if(Gw->R==0)return b("can't merge dc and ac");
if(Gw->T==0){int G1=Gw->U;if(Gw->V){--Gw->V;return 1;}G0=Gw->R;do{unsigned int//
G2;int G3,G4,G5;if(Gw->N<16)AQ(Gw);G3=(Gw->M>>23)&511;G4=Gz[G3];if(G4){G0+=(G4>>
4)&15;G5=G4&15;if(G5>Gw->N)return b("bad huffman code");Gw->M<<=G5;Gw->N-=G5;G2=
AX[G0++];Gx[G2]=(short)((G4>>8)*(1<<G1));}else{int G6=AS(Gw,Gy);if(G6<0)return b
("bad huffman code");G5=G6&15;G4=G6>>4;if(G5==0){if(G4<15){Gw->V=(1<<G4);if(G4)
Gw->V+=AV(Gw,G4);--Gw->V;break;}G0+=16;}else{G0+=G4;G2=AX[G0++];Gx[G2]=(short)(
AU(Gw,G5)*(1<<G1));}}}while(G0<=Gw->S);}else{short G1=(short)(1<<Gw->U);if(Gw->V
){--Gw->V;for(G0=Gw->R;G0<=Gw->S;++G0){short*G2=&Gx[AX[G0]];if(*G2!=0)if(AW(Gw))
if((*G2&G1)==0)if(*G2>0)*G2+=G1;else*G2-=G1;}}else{G0=Gw->R;do{int G2,G3;int G4=
AS(Gw,Gy);if(G4<0)return b("bad huffman code");G3=G4&15;G2=G4>>4;if(G3==0){if(G2
<15){Gw->V=(1<<G2)-1;if(G2)Gw->V+=AV(Gw,G2);G2=64;}else{}}else{if(G3!=1)return b
("bad huffman code");if(AW(Gw))G3=G1;else G3=-G1;}while(G0<=Gw->S){short*G5=&Gx[
AX[G0++]];if(*G5!=0){if(AW(Gw))if((*G5&G1)==0)if(*G5>0)*G5+=G1;else*G5-=G1;}else
{if(G2==0){*G5=(short)G3;break;}--G2;}}}while(G0<=Gw->S);}}return 1;}static A Ab
(int Gw){if((unsigned int)Gw>255){if(Gw<0)return 0;if(Gw>255)return 255;}return(
A)Gw;}static void Ac(A*Gw,int Gx,short Gy[64]){int Gz,G0[64],*G1=G0;A*G2;short*
G3=Gy;for(Gz=0;Gz<8;++Gz,++G3,++G1)if(G3[8]==0&&G3[16]==0&&G3[24]==0&&G3[32]==0
&&G3[40]==0&&G3[48]==0&&G3[56]==0){int G4=G3[0]*4;G1[0]=G1[8]=G1[16]=G1[24]=G1[
32]=G1[40]=G1[48]=G1[56]=G4;}else{int G4,G5,G6,G7,G8,G9,G_,HA,HB,HC,HD,HE,HF;G9=
G3[16];G_=G3[48];G8=(G9+G_)*0.5;G6=G8+G_*-4095.5;G7=G8+G9*0.5;G9=G3[0];G_=G3[32]
;G4=((G9+G_)*4096);G5=((G9-G_)*4096);HC=G4+G7;HF=G4-G7;HD=G5+G6;HE=G5-G6;G4=G3[
56];G5=G3[40];G6=G3[24];G7=G3[8];G_=G4+G6;HA=G5+G7;G8=G4+G7;G9=G5+G6;HB=(G_+HA)*
4096.5;G4=G4*0.5;G5=G5*8192.5;G6=G6*12288.5;G7=G7*4096.5;G8=HB+G8*0.5;G9=HB+G9*-
8191.5;G_=G_*-4095.5;HA=HA*0.5;G7+=G8+HA;G6+=G9+G_;G5+=G9+HA;G4+=G8+G_;HC+=512;
HD+=512;HE+=512;HF+=512;G1[0]=(HC+G7)>>10;G1[56]=(HC-G7)>>10;G1[8]=(HD+G6)>>10;
G1[48]=(HD-G6)>>10;G1[16]=(HE+G5)>>10;G1[40]=(HE-G5)>>10;G1[24]=(HF+G4)>>10;G1[
32]=(HF-G4)>>10;}for(Gz=0,G1=G0,G2=Gw;Gz<8;++Gz,G1+=8,G2+=Gx){int G4,G5,G6,G7,G8
,G9,G_,HA,HB,HC,HD,HE,HF;G9=G1[2];G_=G1[6];G8=(G9+G_)*0.5;G6=G8+G_*-4095.5;G7=G8
+G9*0.5;G9=G1[0];G_=G1[4];G4=((G9+G_)*4096);G5=((G9-G_)*4096);HC=G4+G7;HF=G4-G7;
HD=G5+G6;HE=G5-G6;G4=G1[7];G5=G1[5];G6=G1[3];G7=G1[1];G_=G4+G6;HA=G5+G7;G8=G4+G7
;G9=G5+G6;HB=(G_+HA)*4096.5;G4=G4*0.5;G5=G5*8192.5;G6=G6*12288.5;G7=G7*4096.5;G8
=HB+G8*0.5;G9=HB+G9*-8191.5;G_=G_*-4095.5;HA=HA*0.5;G7+=G8+HA;G6+=G9+G_;G5+=G9+
HA;G4+=G8+G_;HC+=16842752;HD+=16842752;HE+=16842752;HF+=16842752;G2[0]=Ab((HC+G7
)>>17);G2[7]=Ab((HC-G7)>>17);G2[1]=Ab((HD+G6)>>17);G2[6]=Ab((HD-G6)>>17);G2[2]=
Ab((HE+G5)>>17);G2[5]=Ab((HE-G5)>>17);G2[3]=Ab((HF+G4)>>17);G2[4]=Ab((HF-G4)>>17
);}}static A Ad(AN*Gw){A Gx;if(Gw->O!=0xff){Gx=Gw->O;Gw->O=0xff;return Gx;}Gx=AC
(Gw->A);if(Gx!=0xff)return 0xff;while(Gx==0xff)Gx=AC(Gw->A);return Gx;}static///
void Ae(AN*Gw){Gw->N=0;Gw->M=0;Gw->P=0;Gw->L[0].G=Gw->L[1].G=Gw->L[2].G=Gw->L[3]
.G=0;Gw->O=0xff;Gw->c=Gw->b?Gw->b:0x7fffffff;Gw->V=0;}static int Af(AN*Gw){Ae(Gw
);if(!Gw->Q){if(Gw->Z==1){int Gx,Gy;short Gz[64];int G0=Gw->a[0];int G1=(Gw->L[
G0].H+7)>>3;int G2=(Gw->L[G0].I+7)>>3;for(Gy=0;Gy<G2;++Gy)for(Gx=0;Gx<G1;++Gx){
int G3=Gw->L[G0].F;if(!AY(Gw,Gz,Gw->B+Gw->L[G0].E,Gw->C+G3,Gw->E[G3],G0,Gw->D[Gw
->L[G0].D]))return 0;Gw->d(Gw->L[G0].L+Gw->L[G0].J*Gy*8+Gx*8,Gw->L[G0].J,Gz);if(
--Gw->c<=0){if(Gw->N<24)AQ(Gw);if(!(Gw->O>=0xd0&&Gw->O<=0xd7))return 1;Ae(Gw);}}
return 1;}else{int Gx,Gy,Gz,G0,G1;short G2[64];for(Gy=0;Gy<Gw->I;++Gy)for(Gx=0;
Gx<Gw->H;++Gx){for(Gz=0;Gz<Gw->Z;++Gz){int G3=Gw->a[Gz];for(G1=0;G1<Gw->L[G3].C;
++G1)for(G0=0;G0<Gw->L[G3].B;++G0){int G4=(Gx*Gw->L[G3].B+G0)*8;int G5=(Gy*Gw->L
[G3].C+G1)*8;int G6=Gw->L[G3].F;if(!AY(Gw,G2,Gw->B+Gw->L[G3].E,Gw->C+G6,Gw->E[G6
],G3,Gw->D[Gw->L[G3].D]))return 0;Gw->d(Gw->L[G3].L+Gw->L[G3].J*G5+G4,Gw->L[G3].
J,G2);}}if(--Gw->c<=0){if(Gw->N<24)AQ(Gw);if(!(Gw->O>=0xd0&&Gw->O<=0xd7))return
1;Ae(Gw);}}return 1;}}else if(Gw->Z==1){int Gx,Gy;int Gz=Gw->a[0];int G0=(Gw->L[
Gz].H+7)>>3;int G1=(Gw->L[Gz].I+7)>>3;for(Gy=0;Gy<G1;++Gy)for(Gx=0;Gx<G0;++Gx){
short*G2=Gw->L[Gz].P+64*(Gx+Gy*Gw->L[Gz].Q);if(Gw->R==0){if(!AZ(Gw,G2,&Gw->B[Gw
->L[Gz].E],Gz))return 0;}else{int G3=Gw->L[Gz].F;if(!Aa(Gw,G2,&Gw->C[G3],Gw->E[
G3]))return 0;}if(--Gw->c<=0){if(Gw->N<24)AQ(Gw);if(!(Gw->O>=0xd0&&Gw->O<=0xd7))
return 1;Ae(Gw);}}return 1;}else{int Gx,Gy,Gz,G0,G1;for(Gy=0;Gy<Gw->I;++Gy)for(
Gx=0;Gx<Gw->H;++Gx){for(Gz=0;Gz<Gw->Z;++Gz){int G2=Gw->a[Gz];for(G1=0;G1<Gw->L[
G2].C;++G1)for(G0=0;G0<Gw->L[G2].B;++G0){int G3=Gx*Gw->L[G2].B+G0;int G4=Gy*Gw->
L[G2].C+G1;short*G5=Gw->L[G2].P+64*(G3+G4*Gw->L[G2].Q);if(!AZ(Gw,G5,&Gw->B[Gw->L
[G2].E],G2))return 0;}}if(--Gw->c<=0){if(Gw->N<24)AQ(Gw);if(!(Gw->O>=0xd0&&Gw->O
<=0xd7))return 1;Ae(Gw);}}return 1;}}static void Ag(short*Gw,D*Gx){int Gy;for(Gy
=0;Gy<64;++Gy)Gw[Gy]*=Gx[Gy];}static void Ah(AN*Gw){if(Gw->Q){int Gx,Gy,Gz;for(
Gz=0;Gz<Gw->A->C;++Gz){int G0=(Gw->L[Gz].H+7)>>3;int G1=(Gw->L[Gz].I+7)>>3;for(
Gy=0;Gy<G1;++Gy)for(Gx=0;Gx<G0;++Gx){short*G2=Gw->L[Gz].P+64*(Gx+Gy*Gw->L[Gz].Q)
;Ag(G2,Gw->D[Gw->L[Gz].D]);Gw->d(Gw->L[Gz].L+Gw->L[Gz].J*Gy*8+Gx*8,Gw->L[Gz].J,
G2);}}}}static int Ai(AN*Gw,int Gx){int Gy;switch(Gx){case 0xff:return b(///////
"expected marker");case 0xDD:if(AG(Gw->A)!=4)return b("bad DRI len");Gw->b=AG(Gw
->A);return 1;case 0xDB:Gy=AG(Gw->A)-2;while(Gy>0){int Gz=AC(Gw->A);int G0=Gz>>4
,G1=G0!=0;int G2=Gz&15,G3;if(G0!=0&&G0!=1)return b("bad DQT type");if(G2>3)/////
return b("bad DQT table");for(G3=0;G3<64;++G3)Gw->D[G2][AX[G3]]=(D)(G1?AG(Gw->A)
:AC(Gw->A));Gy-=(G1?129:65);}return Gy==0;case 0xC4:Gy=AG(Gw->A)-2;while(Gy>0){A
*Gz;int G0[16],G1,G2=0;int G3=AC(Gw->A);int G4=G3>>4;int G5=G3&15;if(G4>1||G5>3)
return b("bad DHT header");for(G1=0;G1<16;++G1){G0[G1]=AC(Gw->A);G2+=G0[G1];}if(
G2>256)return b("bad DHT header");Gy-=17;if(G4==0){if(!AO(Gw->B+G5,G0))return 0;
Gz=Gw->B[G5].C;}else{if(!AO(Gw->C+G5,G0))return 0;Gz=Gw->C[G5].C;}for(G1=0;G1<G2
;++G1)Gz[G1]=AC(Gw->A);if(G4!=0)AP(Gw->E[G5],Gw->C+G5);Gy-=G2;}return Gy==0;}if(
(Gx>=0xE0&&Gx<=0xEF)||Gx==0xFE){Gy=AG(Gw->A);if(Gy<2)if(Gx==0xFE)return b(//////
"bad COM len");else return b("bad APP len");Gy-=2;if(Gx==0xE0&&Gy>=5){static////
const unsigned char Gz[5]={'J','F','I','F','\0'};int G0=1;int G1;for(G1=0;G1<5;
++G1)if(AC(Gw->A)!=Gz[G1])G0=0;Gy-=5;if(G0)Gw->W=1;}else if(Gx==0xEE&&Gy>=12){//
static const unsigned char Gz[6]={'A','d','o','b','e','\0'};int G0=1;int G1;for(
G1=0;G1<6;++G1)if(AC(Gw->A)!=Gz[G1])G0=0;Gy-=6;if(G0){AC(Gw->A);AG(Gw->A);AG(Gw
->A);Gw->X=AC(Gw->A);Gy-=6;}}AE(Gw->A,Gy);return 1;}return b("unknown marker");}
static int Aj(AN*Gw){int Gx;int Gy=AG(Gw->A);Gw->Z=AC(Gw->A);if(Gw->Z<1||Gw->Z>4
||Gw->Z>(int)Gw->A->C)return b("bad SOS component count");if(Gy!=6+2*Gw->Z)/////
return b("bad SOS len");for(Gx=0;Gx<Gw->Z;++Gx){int Gz=AC(Gw->A),G0;int G1=AC(Gw
->A);for(G0=0;G0<Gw->A->C;++G0)if(Gw->L[G0].A==Gz)break;if(G0==Gw->A->C)return 0
;Gw->L[G0].E=G1>>4;if(Gw->L[G0].E>3)return b("bad DC huff");Gw->L[G0].F=G1&15;if
(Gw->L[G0].F>3)return b("bad AC huff");Gw->a[Gx]=G0;}{int Gz;Gw->R=AC(Gw->A);Gw
->S=AC(Gw->A);Gz=AC(Gw->A);Gw->T=(Gz>>4);Gw->U=(Gz&15);if(Gw->Q){if(Gw->R>63||Gw
->S>63||Gw->R>Gw->S||Gw->T>13||Gw->U>13)return b("bad SOS");}else{if(Gw->R!=0)//
return b("bad SOS");if(Gw->T!=0||Gw->U!=0)return b("bad SOS");Gw->S=63;}}return
1;}static int Ak(AN*Gw,int Gx,int Gy){int Gz;for(Gz=0;Gz<Gx;++Gz){if(Gw->L[Gz].M
){free(Gw->L[Gz].M);Gw->L[Gz].M=0;Gw->L[Gz].L=0;}if(Gw->L[Gz].N){free(Gw->L[Gz].
N);Gw->L[Gz].N=0;Gw->L[Gz].P=0;}if(Gw->L[Gz].O){free(Gw->L[Gz].O);Gw->L[Gz].O=0;
}}return Gy;}static int Al(AN*Gw,int Gx){I*Gy=Gw->A;int Gz,G0,G1,G2,G3=1,G4=1,G5
;Gz=AG(Gy);if(Gz<11)return b("bad SOF len");G0=AC(Gy);if(G0!=8)return b(////////
"only 8-bit");Gy->B=AG(Gy);if(Gy->B==0)return b("no header height");Gy->A=AG(Gy)
;if(Gy->A==0)return b("0 width");if(Gy->B>16777216)return b("too large");if(Gy->
A>16777216)return b("too large");G5=AC(Gy);if(G5!=3&&G5!=1&&G5!=4)return b(/////
"bad component count");Gy->C=G5;for(G1=0;G1<G5;++G1){Gw->L[G1].L=0;Gw->L[G1].O=0
;}if(Gz!=8+3*Gy->C)return b("bad SOF len");Gw->Y=0;for(G1=0;G1<Gy->C;++G1){/////
static const unsigned char G6[3]={'R','G','B'};Gw->L[G1].A=AC(Gy);if(Gy->C==3&&
Gw->L[G1].A==G6[G1])++Gw->Y;G2=AC(Gy);Gw->L[G1].B=(G2>>4);if(!Gw->L[G1].B||Gw->L
[G1].B>4)return b("bad H");Gw->L[G1].C=G2&15;if(!Gw->L[G1].C||Gw->L[G1].C>4)////
return b("bad V");Gw->L[G1].D=AC(Gy);if(Gw->L[G1].D>3)return b("bad TQ");}if(Gx
!=STBI__SCAN_load)return 1;if(!g(Gy->A,Gy->B,Gy->C,0))return b("too large");for(
G1=0;G1<Gy->C;++G1){if(Gw->L[G1].B>G3)G3=Gw->L[G1].B;if(Gw->L[G1].C>G4)G4=Gw->L[
G1].C;}for(G1=0;G1<Gy->C;++G1){if(G3%Gw->L[G1].B!=0)return b("bad H");if(G4%Gw->
L[G1].C!=0)return b("bad V");}Gw->F=G3;Gw->G=G4;Gw->J=G3*8;Gw->K=G4*8;Gw->H=(Gy
->A+Gw->J-1)/Gw->J;Gw->I=(Gy->B+Gw->K-1)/Gw->K;for(G1=0;G1<Gy->C;++G1){Gw->L[G1]
.H=(Gy->A*Gw->L[G1].B+G3-1)/G3;Gw->L[G1].I=(Gy->B*Gw->L[G1].C+G4-1)/G4;Gw->L[G1]
.J=Gw->H*Gw->L[G1].B*8;Gw->L[G1].K=Gw->I*Gw->L[G1].C*8;Gw->L[G1].P=0;Gw->L[G1].N
=0;Gw->L[G1].O=0;Gw->L[G1].M=i(Gw->L[G1].J,Gw->L[G1].K,15);if(Gw->L[G1].M==0)///
return Ak(Gw,G1+1,b("outofmem"));Gw->L[G1].L=(A*)(((size_t)Gw->L[G1].M+15)&~15);
if(Gw->Q){Gw->L[G1].Q=Gw->L[G1].J/8;Gw->L[G1].R=Gw->L[G1].K/8;Gw->L[G1].N=j(Gw->
L[G1].J,Gw->L[G1].K,2,15);if(Gw->L[G1].N==0)return Ak(Gw,G1+1,b("outofmem"));Gw
->L[G1].P=(short*)(((size_t)Gw->L[G1].N+15)&~15);}}return 1;}static int Am(AN*Gw
,int Gx){int Gy;Gw->W=0;Gw->X=-1;Gw->O=0xff;Gy=Ad(Gw);if(!(Gy==0xd8))return b(//
"no SOI");if(Gx==STBI__SCAN_type)return 1;Gy=Ad(Gw);while(!(Gy==0xc0||Gy==0xc1||
Gy==0xc2)){if(!Ai(Gw,Gy))return 0;Gy=Ad(Gw);while(Gy==0xff){if(AD(Gw->A))return
b("no SOF");Gy=Ad(Gw);}}Gw->Q=(Gy==0xc2);if(!Al(Gw,Gx))return 0;return 1;}static
A An(AN*Gw){while(!AD(Gw->A)){A Gx=AC(Gw->A);while(Gx==0xff){if(AD(Gw->A))return
0xff;Gx=AC(Gw->A);if(Gx!=0&&Gx!=0xff)return Gx;}}return 0xff;}static int Ao(AN*
Gw){int Gx;for(Gx=0;Gx<4;Gx++){Gw->L[Gx].M=0;Gw->L[Gx].N=0;}Gw->b=0;if(!Am(Gw,//
STBI__SCAN_load))return 0;Gx=Ad(Gw);while(!(Gx==0xd9))if(Gx==0xda){if(!Aj(Gw))//
return 0;if(!Af(Gw))return 0;if(Gw->O==0xff)Gw->O=An(Gw);Gx=Ad(Gw);if(Gx>=0xd0&&
Gx<=0xd7)Gx=Ad(Gw);}else if(Gx==0xdc){int Gy=AG(Gw->A);F Gz=AG(Gw->A);if(Gy!=4)
return b("bad DNL len");if(Gz!=Gw->A->B)return b("bad DNL height");Gx=Ad(Gw);}//
else{if(!Ai(Gw,Gx))return 1;Gx=Ad(Gw);}if(Gw->Q)Ah(Gw);return 1;}typedef A*(*Ap)
(A*out,A*in0,A*in1,int w,int hs);static A*Aq(A*Gw,A*Gx,A*Gy,int Gz,int G0){/////
return Gx;}static A*Ar(A*Gw,A*Gx,A*Gy,int Gz,int G0){int G1;for(G1=0;G1<Gz;++G1)
Gw[G1]=((A)((3*Gx[G1]+Gy[G1]+2)>>2));return Gw;}static A*As(A*Gw,A*Gx,A*Gy,int//
Gz,int G0){int G1;A*G2=Gx;if(Gz==1){Gw[0]=Gw[1]=G2[0];return Gw;}Gw[0]=G2[0];Gw[
1]=((A)((G2[0]*3+G2[1]+2)>>2));for(G1=1;G1<Gz-1;++G1){int G3=3*G2[G1]+2;Gw[G1*2+
0]=((A)((G3+G2[G1-1])>>2));Gw[G1*2+1]=((A)((G3+G2[G1+1])>>2));}Gw[G1*2+0]=((A)((
G2[Gz-2]*3+G2[Gz-1]+2)>>2));Gw[G1*2+1]=G2[Gz-1];return Gw;}static A*At(A*Gw,A*Gx
,A*Gy,int Gz,int G0){int G1,G2,G3;if(Gz==1){Gw[0]=Gw[1]=((A)((3*Gx[0]+Gy[0]+2)>>
2));return Gw;}G3=3*Gx[0]+Gy[0];Gw[0]=((A)((G3+2)>>2));for(G1=1;G1<Gz;++G1){G2=
G3;G3=3*Gx[G1]+Gy[G1];Gw[G1*2-1]=((A)((3*G2+G3+8)>>4));Gw[G1*2]=((A)((3*G3+G2+8)
>>4));}Gw[Gz*2-1]=((A)((G3+2)>>2));return Gw;}static A*Au(A*Gw,A*Gx,A*Gy,int Gz,
int G0){int G1,G2;for(G1=0;G1<Gz;++G1)for(G2=0;G2<G0;++G2)Gw[G1*G0+G2]=Gx[G1];//
return Gw;}static void Av(A*Gw,const A*Gx,const A*Gy,const A*Gz,int G0,int G1){
int G2;for(G2=0;G2<G0;++G2){int G3=(Gx[G2]<<20)+524288;int G4,G5,G6;int G7=Gz[G2
]-128;int G8=Gy[G2]-128;G4=G3+G7*1470208;G5=G3+(G7*-748800)+((G8*-360960)&//////
0xffff0000);G6=G3+G8*1858048;G4>>=20;G5>>=20;G6>>=20;if((unsigned)G4>255)if(G4<0
)G4=0;else G4=255;if((unsigned)G5>255)if(G5<0)G5=0;else G5=255;if((unsigned)G6>
255)if(G6<0)G6=0;else G6=255;Gw[0]=(A)G4;Gw[1]=(A)G5;Gw[2]=(A)G6;Gw[3]=255;Gw+=
G1;}}static void Aw(AN*Gw){Gw->d=Ac;Gw->e=Av;Gw->f=At;}static void Ax(AN*Gw){Ak(
Gw,Gw->A->C,0);}typedef struct{Ap A;A*B,*C;int D,E;int F;int G;int H;}Ay;static
A Az(A Gw,A Gx){unsigned int Gy=Gw*Gx+128;return(A)((Gy+(Gy>>8))>>8);}static A*
A0(AN*Gw,int*Gx,int*Gy,int*Gz,int G0){int G1,G2,G3;Gw->A->C=0;if(G0<0||G0>4)////
return(unsigned char*)(size_t)(b("bad req_comp")?0:0);if(!Ao(Gw)){Ax(Gw);return
0;}G1=G0?G0:Gw->A->C>=3?3:1;G3=Gw->A->C==3&&(Gw->Y==3||(Gw->X==0&&!Gw->W));if(Gw
->A->C==3&&G1<3&&!G3)G2=1;else G2=Gw->A->C;if(G2<=0){Ax(Gw);return 0;}{int G4;//
unsigned int G5,G6;A*G7;A*G8[4]={0,0,0,0};Ay G9[4];for(G4=0;G4<G2;++G4){Ay*G_=&
G9[G4];Gw->L[G4].O=(A*)c(Gw->A->A+3);if(!Gw->L[G4].O){Ax(Gw);return(unsigned////
char*)(size_t)(b("outofmem")?0:0);}G_->D=Gw->F/Gw->L[G4].B;G_->E=Gw->G/Gw->L[G4]
.C;G_->G=G_->E>>1;G_->F=(Gw->A->A+G_->D-1)/G_->D;G_->H=0;G_->B=G_->C=Gw->L[G4].L
;if(G_->D==1&&G_->E==1)G_->A=Aq;else if(G_->D==1&&G_->E==2)G_->A=Ar;else if(G_->
D==2&&G_->E==1)G_->A=As;else if(G_->D==2&&G_->E==2)G_->A=Gw->f;else G_->A=Au;}G7
=(A*)j(G1,Gw->A->A,Gw->A->B,1);if(!G7){Ax(Gw);return(unsigned char*)(size_t)(b(
"outofmem")?0:0);}for(G6=0;G6<Gw->A->B;++G6){A*G_=G7+G1*Gw->A->A*G6;for(G4=0;G4<
G2;++G4){Ay*HA=&G9[G4];int HB=HA->G>=(HA->E>>1);G8[G4]=HA->A(Gw->L[G4].O,HB?HA->
C:HA->B,HB?HA->B:HA->C,HA->F,HA->D);if(++HA->G>=HA->E){HA->G=0;HA->B=HA->C;if(++
HA->H<Gw->L[G4].I)HA->C+=Gw->L[G4].J;}}if(G1>=3){A*HA=G8[0];if(Gw->A->C==3){if(
G3)for(G5=0;G5<Gw->A->A;++G5){G_[0]=HA[G5];G_[1]=G8[1][G5];G_[2]=G8[2][G5];G_[3]
=255;G_+=G1;}else Gw->e(G_,HA,G8[1],G8[2],Gw->A->A,G1);}else if(Gw->A->C==4){if(
Gw->X==0)for(G5=0;G5<Gw->A->A;++G5){A HB=G8[3][G5];G_[0]=Az(G8[0][G5],HB);G_[1]=
Az(G8[1][G5],HB);G_[2]=Az(G8[2][G5],HB);G_[3]=255;G_+=G1;}else if(Gw->X==2){Gw->
e(G_,HA,G8[1],G8[2],Gw->A->A,G1);for(G5=0;G5<Gw->A->A;++G5){A HB=G8[3][G5];G_[0]
=Az(255-G_[0],HB);G_[1]=Az(255-G_[1],HB);G_[2]=Az(255-G_[2],HB);G_+=G1;}}else Gw
->e(G_,HA,G8[1],G8[2],Gw->A->A,G1);}else for(G5=0;G5<Gw->A->A;++G5){G_[0]=G_[1]=
G_[2]=HA[G5];G_[3]=255;G_+=G1;}}else if(G3){if(G1==1)for(G5=0;G5<Gw->A->A;++G5)*
G_++ =AI(G8[0][G5],G8[1][G5],G8[2][G5]);else for(G5=0;G5<Gw->A->A;++G5,G_+=2){G_
[0]=AI(G8[0][G5],G8[1][G5],G8[2][G5]);G_[1]=255;}}else if(Gw->A->C==4&&Gw->X==0)
for(G5=0;G5<Gw->A->A;++G5){A HA=G8[3][G5];A HB=Az(G8[0][G5],HA);A HC=Az(G8[1][G5
],HA);A HD=Az(G8[2][G5],HA);G_[0]=AI(HB,HC,HD);G_[1]=255;G_+=G1;}else if(Gw->A->
C==4&&Gw->X==2)for(G5=0;G5<Gw->A->A;++G5){G_[0]=Az(255-G8[0][G5],G8[3][G5]);G_[1
]=255;G_+=G1;}else{A*HA=G8[0];if(G1==1)for(G5=0;G5<Gw->A->A;++G5)G_[G5]=HA[G5];
else for(G5=0;G5<Gw->A->A;++G5){*G_++ =HA[G5];*G_++ =255;}}}Ax(Gw);*Gx=Gw->A->A;
*Gy=Gw->A->B;if(Gz)*Gz=Gw->A->C>=3?3:1;return G7;}}static void*U(I*G2,int*G3,int
*G4,int*G5,int G6,S*G7){unsigned char*G8;AN*G9=(AN*)c(18568);if(!G9)return((////
unsigned char*)(size_t)(b("outofmem")?0:0));memset(G9,0,18568);G9->A=G2;Aw(G9);
G8=A0(G9,G3,G4,G5,G6);free(G9);return G8;}static int T(I*Gx){int Gy;AN*Gz=(AN*)c
(18568);if(!Gz)return b("outofmem");memset(Gz,0,18568);Gz->A=Gx;Aw(Gz);Gy=Am(Gz,
STBI__SCAN_type);R(Gx);free(Gz);return Gy;}static int A1(AN*Gw,int*Gx,int*Gy,int
*Gz){if(!Am(Gw,STBI__SCAN_header)){R(Gw->A);return 0;}if(Gx)*Gx=Gw->A->A;if(Gy)*
Gy=Gw->A->B;if(Gz)*Gz=Gw->A->C>=3?3:1;return 1;}static int V(I*G0,int*G1,int*G2,
int*G3){int G4;AN*G5=(AN*)c(18568);if(!G5)return b("outofmem");memset(G5,0,18568
);G5->A=G0;G4=A1(G5,G1,G2,G3);free(G5);return G4;}typedef struct{D A[512];D B[16
];int C[17];D D[16];A E[288];D F[288];}A2;static int A3(int Gw){Gw=((Gw&0xAAAA)
>>1)|((Gw&0x5555)<<1);Gw=((Gw&0xCCCC)>>2)|((Gw&0x3333)<<2);Gw=((Gw&0xF0F0)>>4)|(
(Gw&3855)<<4);Gw=((Gw&0xFF00)>>8)|((Gw&255)<<8);return Gw;}static int A4(int Gw,
int Gx){return A3(Gw)>>(16-Gx);}static int A5(A2*Gw,const A*Gx,int Gy){int Gz,G0
=0;int G1,G2[16],G3[17];memset(G3,0,68);memset(Gw->A,0,1024);for(Gz=0;Gz<Gy;++Gz
)++G3[Gx[Gz]];G3[0]=0;for(Gz=1;Gz<16;++Gz)if(G3[Gz]>(1<<Gz))return b("bad sizes"
);G1=0;for(Gz=1;Gz<16;++Gz){G2[Gz]=G1;Gw->B[Gz]=(D)G1;Gw->D[Gz]=(D)G0;G1=(G1+G3[
Gz]);if(G3[Gz])if(G1-1>=(1<<Gz))return b("bad codelengths");Gw->C[Gz]=G1<<(16-Gz
);G1<<=1;G0+=G3[Gz];}Gw->C[16]=65536;for(Gz=0;Gz<Gy;++Gz){int G4=Gx[Gz];if(G4){
int G5=G2[G4]-Gw->B[G4]+Gw->D[G4];D G6=(D)((G4<<9)|Gz);Gw->E[G5]=(A)G4;Gw->F[G5]
=(D)Gz;if(G4<=9){int G7=A4(G2[G4],G4);while(G7<512){Gw->A[G7]=G6;G7+=(1<<G4);}}
++G2[G4];}}return 1;}typedef struct{A*A,*B;int C;int D;F E;char*F;char*G;char*H;
int I;A2 J,K;}A6;static int A7(A6*Gw){return Gw->A>=Gw->B;}static A A8(A6*Gw){//
return A7(Gw)?0:*Gw->A++;}static void A9(A6*Gw){do{if(Gw->E>=(1U<<Gw->C)){Gw->A=
Gw->B;return;}Gw->E|=(unsigned int)A8(Gw)<<Gw->C;Gw->C+=8;}while(Gw->C<=24);}///
static unsigned int A_(A6*Gw,int Gx){unsigned int Gy;if(Gw->C<Gx)A9(Gw);Gy=Gw->E
&((1<<Gx)-1);Gw->E>>=Gx;Gw->C-=Gx;return Gy;}static int BA(A6*Gw,A2*Gx){int Gy,
Gz,G0;G0=A4(Gw->E,16);for(Gz=10;;++Gz)if(G0<Gx->C[Gz])break;if(Gz>=16)return-1;
Gy=(G0>>(16-Gz))-Gx->B[Gz]+Gx->D[Gz];if(Gy>=288)return-1;if(Gx->E[Gy]!=Gz)return
-1;Gw->E>>=Gz;Gw->C-=Gz;return Gx->F[Gy];}static int BB(A6*Gw,A2*Gx){int Gy,Gz;
if(Gw->C<16)if(A7(Gw)){if(!Gw->D){Gw->D=1;Gw->C+=16;}else return-1;}else A9(Gw);
Gy=Gx->A[Gw->E&511];if(Gy){Gz=Gy>>9;Gw->E>>=Gz;Gw->C-=Gz;return Gy&511;}return//
BA(Gw,Gx);}static int BC(A6*Gw,char*Gx,int Gy){char*Gz;unsigned int G0,G1,G2;Gw
->F=Gx;if(!Gw->I)return b("output buffer limit");G0=(unsigned int)(Gw->F-Gw->G);
G1=G2=(unsigned)(Gw->H-Gw->G);if(UINT_MAX-G0<(unsigned)Gy)return b("outofmem");
while(G0+Gy>G1){if(G1>2147483647)return b("outofmem");G1*=2;}Gz=(char*)realloc(
Gw->G,G1);if(Gz==0)return b("outofmem");Gw->G=Gz;Gw->F=Gz+G0;Gw->H=Gz+G1;return
1;}static const int BD[31]={3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59
,67,83,99,115,131,163,195,227,258,0,0};static const int BE[31]={0,0,0,0,0,0,0,0,
1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0};static const int BF[32]={1,2,3,4,
5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145
,8193,12289,16385,24577,0,0};static const int BG[32]={0,0,0,0,1,1,2,2,3,3,4,4,5,
5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};static int BH(A6*Gw){char*Gx=Gw->F;//
for(;;){int Gy=BB(Gw,&Gw->J);if(Gy<256){if(Gy<0)return b("bad huffman code");if(
Gx>=Gw->H){if(!BC(Gw,Gx,1))return 0;Gx=Gw->F;}*Gx++ =(char)Gy;}else{A*Gz;int G0,
G1;if(Gy==256){Gw->F=Gx;if(Gw->D&&Gw->C<16)return b("unexpected end");return 1;}
if(Gy>=286)return b("bad huffman code");Gy-=257;G0=BD[Gy];if(BE[Gy])G0+=A_(Gw,BE
[Gy]);Gy=BB(Gw,&Gw->K);if(Gy<0||Gy>=30)return b("bad huffman code");G1=BF[Gy];if
(BG[Gy])G1+=A_(Gw,BG[Gy]);if(Gx-Gw->G<G1)return b("bad dist");if(G0>Gw->H-Gx){if
(!BC(Gw,Gx,G0))return 0;Gx=Gw->F;}Gz=(A*)(Gx-G1);if(G1==1){A G2=*Gz;if(G0)do*Gx
++ =G2;while(--G0);}else if(G0)do*Gx++ =*Gz++;while(--G0);}}}static int BI(A6*Gw
){static const A Gx[19]={16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};A2 Gy;A
Gz[455];A G0[19];int G1,G2;int G3=A_(Gw,5)+257;int G4=A_(Gw,5)+1;int G5=A_(Gw,4)
+4;int G6=G3+G4;memset(G0,0,19);for(G1=0;G1<G5;++G1){int G7=A_(Gw,3);G0[Gx[G1]]=
(A)G7;}if(!A5(&Gy,G0,19))return 0;G2=0;while(G2<G6){int G7=BB(Gw,&Gy);if(G7<0||
G7>=19)return b("bad codelengths");if(G7<16)Gz[G2++]=(A)G7;else{A G8=0;if(G7==16
){G7=A_(Gw,2)+3;if(G2==0)return b("bad codelengths");G8=Gz[G2-1];}else if(G7==17
)G7=A_(Gw,3)+3;else if(G7==18)G7=A_(Gw,7)+11;else return b("bad codelengths");if
(G6-G2<G7)return b("bad codelengths");memset(Gz+G2,G8,G7);G2+=G7;}}if(G2!=G6)///
return b("bad codelengths");if(!A5(&Gw->J,Gz,G3))return 0;if(!A5(&Gw->K,Gz+G3,G4
))return 0;return 1;}static int BJ(A6*Gw){A Gx[4];int Gy,Gz,G0;if(Gw->C&7)A_(Gw,
Gw->C&7);G0=0;while(Gw->C>0){Gx[G0++]=(A)(Gw->E&255);Gw->E>>=8;Gw->C-=8;}if(Gw->
C<0)return b("zlib corrupt");while(G0<4)Gx[G0++]=A8(Gw);Gy=Gx[1]*256+Gx[0];Gz=Gx
[3]*256+Gx[2];if(Gz!=(Gy^0xffff))return b("zlib corrupt");if(Gw->A+Gy>Gw->B)////
return b("read past buffer");if(Gw->F+Gy>Gw->H)if(!BC(Gw,Gw->F,Gy))return 0;////
memcpy(Gw->F,Gw->A,Gy);Gw->A+=Gy;Gw->F+=Gy;return 1;}static int BK(A6*Gw){int Gx
=A8(Gw);int Gy=Gx&15;int Gz=A8(Gw);if(A7(Gw))return b("bad zlib header");if((Gx*
256+Gz)%31!=0)return b("bad zlib header");if(Gz&32)return b("no preset dict");if
(Gy!=8)return b("bad compression");return 1;}static const A BL[288]={8,8,8,8,8,8
,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9
,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9
,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9
,9,9,9,9,9,9,9,9,9,9,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,8,8,8,8,8,8
,8,8};static const A BM[32]={5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5
,5,5,5,5,5,5};static int BN(A6*Gw,int Gx){int Gy,Gz;if(Gx)if(!BK(Gw))return 0;Gw
->C=0;Gw->E=0;Gw->D=0;do{Gy=A_(Gw,1);Gz=A_(Gw,2);if(Gz==0){if(!BJ(Gw))return 0;}
else if(Gz==3)return 0;else{if(Gz==1){if(!A5(&Gw->J,BL,288))return 0;if(!A5(&Gw
->K,BM,32))return 0;}else if(!BI(Gw))return 0;if(!BH(Gw))return 0;}}while(!Gy);
return 1;}static int BO(A6*Gw,char*Gx,int Gy,int Gz,int G0){Gw->G=Gx;Gw->F=Gx;Gw
->H=Gx+Gy;Gw->I=Gz;return BN(Gw,G0);}extern char*///////////////////////////////
stbi_zlib_decode_malloc_guesssize(const char*Gw,int Gx,int Gy,int*Gz){A6 G0;char
*G1=(char*)c(Gy);if(G1==0)return 0;G0.A=(A*)Gw;G0.B=(A*)Gw+Gx;if(BO(&G0,G1,Gy,1,
1)){if(Gz)*Gz=(int)(G0.F-G0.G);return G0.G;}else{free(G0.G);return 0;}}extern///
char*stbi_zlib_decode_malloc(char const*Gw,int Gx,int*Gy){return////////////////
stbi_zlib_decode_malloc_guesssize(Gw,Gx,16384,Gy);}extern char*/////////////////
stbi_zlib_decode_malloc_guesssize_headerflag(const char*Gw,int Gx,int Gy,int*Gz,
int G0){A6 G1;char*G2=(char*)c(Gy);if(G2==0)return 0;G1.A=(A*)Gw;G1.B=(A*)Gw+Gx;
if(BO(&G1,G2,Gy,1,G0)){if(Gz)*Gz=(int)(G1.F-G1.G);return G1.G;}else{free(G1.G);
return 0;}}extern int stbi_zlib_decode_buffer(char*Gw,int Gx,char const*Gy,int//
Gz){A6 G0;G0.A=(A*)Gy;G0.B=(A*)Gy+Gz;if(BO(&G0,Gw,Gx,0,1))return(int)(G0.F-G0.G)
;else return-1;}extern char*stbi_zlib_decode_noheader_malloc(char const*Gw,int//
Gx,int*Gy){A6 Gz;char*G0=(char*)c(16384);if(G0==0)return 0;Gz.A=(A*)Gw;Gz.B=(A*)
Gw+Gx;if(BO(&Gz,G0,16384,1,0)){if(Gy)*Gy=(int)(Gz.F-Gz.G);return Gz.G;}else{free
(Gz.G);return 0;}}extern int stbi_zlib_decode_noheader_buffer(char*Gw,int Gx,///
const char*Gy,int Gz){A6 G0;G0.A=(A*)Gy;G0.B=(A*)Gy+Gz;if(BO(&G0,Gw,Gx,0,0))////
return(int)(G0.F-G0.G);else return-1;}typedef struct{F A;F B;}BP;static BP BQ(I*
Gw){BP Gx;Gx.A=AH(Gw);Gx.B=AH(Gw);return Gx;}static int BR(I*Gw){static const A
Gx[8]={137,80,78,71,13,10,26,10};int Gy;for(Gy=0;Gy<8;++Gy)if(AC(Gw)!=Gx[Gy])///
return b("bad png sig");return 1;}typedef struct{I*A;A*B,*C,*D;int E;}BS;enum{//
STBI__F_none=0,STBI__F_sub=1,STBI__F_up=2,STBI__F_avg=3,STBI__F_paeth=4,////////
STBI__F_avg_first};static A BT[5]={STBI__F_none,STBI__F_sub,STBI__F_none,///////
STBI__F_avg_first,STBI__F_sub};static int BU(int Gw,int Gx,int Gy){int Gz=Gy*3-(
Gw+Gx);int G0=Gw<Gx?Gw:Gx;int G1=Gw<Gx?Gx:Gw;int G2=(G1<=Gz)?G0:Gy;int G3=(Gz<=
G0)?G1:G2;return G3;}static const A BV[9]={0,0xff,85,0,17,0,0,0,1};static void//
BW(A*Gw,A*Gx,F Gy,int Gz){int G0;if(Gz==1)for(G0=Gy-1;G0>=0;--G0){Gw[G0*2+1]=255
;Gw[G0*2+0]=Gx[G0];}else for(G0=Gy-1;G0>=0;--G0){Gw[G0*4+3]=255;Gw[G0*4+2]=Gx[G0
*3+2];Gw[G0*4+1]=Gx[G0*3+1];Gw[G0*4+0]=Gx[G0*3+0];}}static int BX(BS*Gw,A*Gx,F//
Gy,int Gz,F G0,F G1,int G2,int G3){int G4=G2==16?2:1;I*G5=Gw->A;F G6,G7,G8=G0*Gz
*G4;F G9,G_;A*HA;int HB=1;int HC;int HD=G5->C;int HE=Gz*G4;int HF=HD*G4;int HG=
G0;Gw->D=(A*)j(G0,G1,HE,0);if(!Gw->D)return b("outofmem");if(!g(HD,G0,G2,7))////
return b("too large");G_=(((HD*G0*G2)+7)>>3);if(!f(G_,G1,G_))return b(//////////
"too large");G9=(G_+1)*G1;if(Gy<G9)return b("not enough pixels");HA=(A*)i(G_,2,0
);if(!HA)return b("outofmem");if(G2<8){HF=1;HG=G_;}for(G7=0;G7<G1;++G7){A*HH=HA+
(G7&1)*G_;A*HI=HA+(~G7&1)*G_;A*HJ=Gw->D+G8*G7;int HK=HG*HF;int HL=*Gx++;if(HL>4)
{HB=b("invalid filter");break;}if(G7==0)HL=BT[HL];switch(HL){case STBI__F_none:
memcpy(HH,Gx,HK);break;case STBI__F_sub:memcpy(HH,Gx,HF);for(HC=HF;HC<HK;++HC)HH
[HC]=((A)((Gx[HC]+HH[HC-HF])&255));break;case STBI__F_up:for(HC=0;HC<HK;++HC)HH[
HC]=((A)((Gx[HC]+HI[HC])&255));break;case STBI__F_avg:for(HC=0;HC<HF;++HC)HH[HC]
=((A)((Gx[HC]+(HI[HC]>>1))&255));for(HC=HF;HC<HK;++HC)HH[HC]=((A)((Gx[HC]+((HI[
HC]+HH[HC-HF])>>1))&255));break;case STBI__F_paeth:for(HC=0;HC<HF;++HC)HH[HC]=((
A)((Gx[HC]+HI[HC])&255));for(HC=HF;HC<HK;++HC)HH[HC]=((A)((Gx[HC]+BU(HH[HC-HF],
HI[HC],HI[HC-HF]))&255));break;case STBI__F_avg_first:memcpy(HH,Gx,HF);for(HC=HF
;HC<HK;++HC)HH[HC]=((A)((Gx[HC]+(HH[HC-HF]>>1))&255));break;}Gx+=HK;if(G2<8){A//
HM=(G3==0)?BV[G2]:1;A*HN=HH;A*HO=HJ;A HP=0;F HQ=G0*HD;if(G2==4)for(G6=0;G6<HQ;++
G6){if((G6&1)==0)HP=*HN++;*HO++ =HM*(HP>>4);HP<<=4;}else if(G2==2)for(G6=0;G6<HQ
;++G6){if((G6&3)==0)HP=*HN++;*HO++ =HM*(HP>>6);HP<<=2;}else for(G6=0;G6<HQ;++G6)
{if((G6&7)==0)HP=*HN++;*HO++ =HM*(HP>>7);HP<<=1;}if(HD!=Gz)BW(HJ,HJ,G0,HD);}else
if(G2==8){if(HD==Gz)memcpy(HJ,HH,G0*HD);else BW(HJ,HH,G0,HD);}else if(G2==16){D*
HM=(D*)HJ;F HN=G0*HD;if(HD==Gz)for(G6=0;G6<HN;++G6,++HM,HH+=2)*HM=(HH[0]<<8)|HH[
1];else if(HD==1)for(G6=0;G6<G0;++G6,HM+=2,HH+=2){HM[0]=(HH[0]<<8)|HH[1];HM[1]=
0xffff;}else for(G6=0;G6<G0;++G6,HM+=4,HH+=6){HM[0]=(HH[0]<<8)|HH[1];HM[1]=(HH[2
]<<8)|HH[3];HM[2]=(HH[4]<<8)|HH[5];HM[3]=0xffff;}}}free(HA);if(!HB)return 0;////
return 1;}static int BY(BS*Gw,A*Gx,F Gy,int Gz,int G0,int G1,int G2){int G3=G0==
16?2:1;int G4=Gz*G3;A*G5;int G6;if(!G2)return BX(Gw,Gx,Gy,Gz,Gw->A->A,Gw->A->B,
G0,G1);G5=(A*)j(Gw->A->A,Gw->A->B,G4,0);if(!G5)return b("outofmem");for(G6=0;G6<
7;++G6){int G7[]={0,4,0,2,0,1,0};int G8[]={0,0,4,0,2,0,1};int G9[]={8,8,4,4,2,2,
1};int G_[]={8,8,8,4,4,2,2};int HA,HB,HC,HD;HC=(Gw->A->A-G7[G6]+G9[G6]-1)/G9[G6]
;HD=(Gw->A->B-G8[G6]+G_[G6]-1)/G_[G6];if(HC&&HD){F HE=((((Gw->A->C*HC*G0)+7)>>3)
+1)*HD;if(!BX(Gw,Gx,Gy,Gz,HC,HD,G0,G1)){free(G5);return 0;}for(HB=0;HB<HD;++HB)
for(HA=0;HA<HC;++HA){int HF=HB*G_[G6]+G8[G6];int HG=HA*G9[G6]+G7[G6];memcpy(G5+
HF*Gw->A->A*G4+HG*G4,Gw->D+(HB*HC+HA)*G4,G4);}free(Gw->D);Gx+=HE;Gy-=HE;}}Gw->D=
G5;return 1;}static int BZ(BS*Gw,A Gx[3],int Gy){I*Gz=Gw->A;F G0,G1=Gz->A*Gz->B;
A*G2=Gw->D;if(Gy==2)for(G0=0;G0<G1;++G0){G2[1]=(G2[0]==Gx[0]?0:255);G2+=2;}else
for(G0=0;G0<G1;++G0){if(G2[0]==Gx[0]&&G2[1]==Gx[1]&&G2[2]==Gx[2])G2[3]=0;G2+=4;}
return 1;}static int Ba(BS*Gw,D Gx[3],int Gy){I*Gz=Gw->A;F G0,G1=Gz->A*Gz->B;D*
G2=(D*)Gw->D;if(Gy==2)for(G0=0;G0<G1;++G0){G2[1]=(G2[0]==Gx[0]?0:65535);G2+=2;}
else for(G0=0;G0<G1;++G0){if(G2[0]==Gx[0]&&G2[1]==Gx[1]&&G2[2]==Gx[2])G2[3]=0;G2
+=4;}return 1;}static int Bb(BS*Gw,A*Gx,int Gy,int Gz){F G0,G1=Gw->A->A*Gw->A->B
;A*G2,*G3,*G4=Gw->D;G2=(A*)i(G1,Gz,0);if(G2==0)return b("outofmem");G3=G2;if(Gz
==3)for(G0=0;G0<G1;++G0){int G5=G4[G0]*4;G2[0]=Gx[G5];G2[1]=Gx[G5+1];G2[2]=Gx[G5
+2];G2+=3;}else for(G0=0;G0<G1;++G0){int G5=G4[G0]*4;G2[0]=Gx[G5];G2[1]=Gx[G5+1]
;G2[2]=Gx[G5+2];G2[3]=Gx[G5+3];G2+=4;}free(Gw->D);Gw->D=G3;return 1;}static int
Bc=0;static int Bd=0;extern void stbi_set_unpremultiply_on_load(int Gw){Bc=Gw;}
extern void stbi_convert_iphone_png_to_rgb(int Gw){Bd=Gw;}static _Thread_local//
int Be,Bf;static _Thread_local int Bg,Bh;extern void////////////////////////////
stbi_set_unpremultiply_on_load_thread(int Gw){Be=Gw;Bf=1;}extern void///////////
stbi_convert_iphone_png_to_rgb_thread(int Gw){Bg=Gw;Bh=1;}static void Bi(BS*Gw){
I*Gx=Gw->A;F Gy,Gz=Gx->A*Gx->B;A*G0=Gw->D;if(Gx->D==3)for(Gy=0;Gy<Gz;++Gy){A G1=
G0[0];G0[0]=G0[2];G0[2]=G1;G0+=3;}else if(Bf?Be:Bc)for(Gy=0;Gy<Gz;++Gy){A G1=G0[
3];A G2=G0[0];if(G1){A G3=G1/2;G0[0]=(G0[2]*255+G3)/G1;G0[1]=(G0[1]*255+G3)/G1;
G0[2]=(G2*255+G3)/G1;}else{G0[0]=G0[2];G0[2]=G2;}G0+=4;}else for(Gy=0;Gy<Gz;++Gy
){A G1=G0[0];G0[0]=G0[2];G0[2]=G1;G0+=4;}}static int Bj(BS*Gw,int Gx,int Gy){A//
Gz[1024],G0=0;A G1=0,G2[3]={0};D G3[3];F G4=0,G5=0,G6,G7=0;int G8=1,G9,G_=0,HA=0
,HB=0;I*HC=Gw->A;Gw->C=0;Gw->B=0;Gw->D=0;if(!BR(HC))return 0;if(Gx==////////////
STBI__SCAN_type)return 1;for(;;){BP HD=BQ(HC);switch(HD.B){case 1130840649:HB=1;
AE(HC,HD.A);break;case 1229472850:{int HE,HF;if(!G8)return b("multiple IHDR");G8
=0;if(HD.A!=13)return b("bad IHDR len");HC->A=AH(HC);HC->B=AH(HC);if(HC->B>/////
16777216)return b("too large");if(HC->A>16777216)return b("too large");Gw->E=AC(
HC);if(Gw->E!=1&&Gw->E!=2&&Gw->E!=4&&Gw->E!=8&&Gw->E!=16)return b(//////////////
"1/2/4/8/16-bit only");HA=AC(HC);if(HA>6)return b("bad ctype");if(HA==3&&Gw->E==
16)return b("bad ctype");if(HA==3)G0=3;else if(HA&1)return b("bad ctype");HE=AC(
HC);if(HE)return b("bad comp method");HF=AC(HC);if(HF)return b(/////////////////
"bad filter method");G_=AC(HC);if(G_>1)return b("bad interlace method");if(!HC->
A||!HC->B)return b("0-pixel image");if(!G0){HC->C=(HA&2?3:1)+(HA&4?1:0);if((1<<
30)/HC->A/HC->C<HC->B)return b("too large");}else{HC->C=1;if((1<<30)/HC->A/4<HC
->B)return b("too large");}break;}case 1347179589:{if(G8)return b(//////////////
"first not IHDR");if(HD.A>768)return b("invalid PLTE");G7=HD.A/3;if(G7*3!=HD.A)
return b("invalid PLTE");for(G6=0;G6<G7;++G6){Gz[G6*4+0]=AC(HC);Gz[G6*4+1]=AC(HC
);Gz[G6*4+2]=AC(HC);Gz[G6*4+3]=255;}break;}case 1951551059:{if(G8)return b(/////
"first not IHDR");if(Gw->B)return b("tRNS after IDAT");if(G0){if(Gx==///////////
STBI__SCAN_header){HC->C=4;return 1;}if(G7==0)return b("tRNS before PLTE");if(HD
.A>G7)return b("bad tRNS len");G0=4;for(G6=0;G6<HD.A;++G6)Gz[G6*4+3]=AC(HC);}///
else{if(!(HC->C&1))return b("tRNS with alpha");if(HD.A!=(F)HC->C*2)return b(////
"bad tRNS len");G1=1;if(Gx==STBI__SCAN_header){++HC->C;return 1;}if(Gw->E==16)//
for(G9=0;G9<HC->C&&G9<3;++G9)G3[G9]=(D)AG(HC);else for(G9=0;G9<HC->C&&G9<3;++G9)
G2[G9]=(A)(AG(HC)&255)*BV[Gw->E];}break;}case 1229209940:{if(G8)return b(///////
"first not IHDR");if(G0&&!G7)return b("no PLTE");if(Gx==STBI__SCAN_header){if(G0
)HC->C=G0;return 1;}if(HD.A>(1u<<30))return b("IDAT size limit");if((int)(G4+HD.
A)<(int)G4)return 0;if(G4+HD.A>G5){F HE=G5;A*HF;if(G5==0)G5=HD.A>4096?HD.A:4096;
while(G4+HD.A>G5)G5*=2;HF=(A*)realloc(Gw->B,G5);if(HF==0)return b("outofmem");Gw
->B=HF;}if(!AF(HC,Gw->B+G4,HD.A))return b("outofdata");G4+=HD.A;break;}case/////
1229278788:{F HE,HF;if(G8)return b("first not IHDR");if(Gx!=STBI__SCAN_load)////
return 1;if(Gw->B==0)return b("no IDAT");HF=(HC->A*Gw->E+7)/8;HE=HF*HC->B*HC->C+
HC->B;Gw->C=(A*)stbi_zlib_decode_malloc_guesssize_headerflag((char*)Gw->B,G4,HE,
(int*)&HE,!HB);if(Gw->C==0)return 0;free(Gw->B);Gw->B=0;if((Gy==HC->C+1&&Gy!=3&&
!G0)||G1)HC->D=HC->C+1;else HC->D=HC->C;if(!BY(Gw,Gw->C,HE,HC->D,Gw->E,HA,G_))//
return 0;if(G1)if(Gw->E==16){if(!Ba(Gw,G3,HC->D))return 0;}else if(!BZ(Gw,G2,HC
->D))return 0;if(HB&&(Bh?Bg:Bd)&&HC->D>2)Bi(Gw);if(G0){HC->C=G0;HC->D=G0;if(Gy>=
3)HC->D=Gy;if(!Bb(Gw,Gz,G7,HC->D))return 0;}else if(G1)++HC->C;free(Gw->C);Gw->C
=0;AH(HC);return 1;}default:if(G8)return b("first not IHDR");if((HD.B&(1<<29))==
0){static char HE[]="XXXX PNG chunk not known";HE[0]=((A)((HD.B>>24)&255));HE[1]
=((A)((HD.B>>16)&255));HE[2]=((A)((HD.B>>8)&255));HE[3]=((A)((HD.B>>0)&255));///
return b(HE);}AE(HC,HD.A);break;}AH(HC);}}static void*Bk(BS*Gw,int*Gx,int*Gy,int
*Gz,int G0,S*G1){void*G2=0;if(G0<0||G0>4)return((unsigned char*)(size_t)(b(/////
"bad req_comp")?0:0));if(Bj(Gw,STBI__SCAN_load,G0)){if(Gw->E<=8)G1->A=8;else if(
Gw->E==16)G1->A=16;else return((unsigned char*)(size_t)(b("bad bits_per_channel"
)?0:0));G2=Gw->D;Gw->D=0;if(G0&&G0!=Gw->A->D){if(G1->A==8)G2=AJ((unsigned char*)
G2,Gw->A->D,G0,Gw->A->A,Gw->A->B);else G2=AL((D*)G2,Gw->A->D,G0,Gw->A->A,Gw->A->
B);Gw->A->D=G0;if(G2==0)return G2;}*Gx=Gw->A->A;*Gy=Gw->A->B;if(Gz)*Gz=Gw->A->C;
}free(Gw->D);Gw->D=0;free(Gw->C);Gw->C=0;free(Gw->B);Gw->B=0;return G2;}static//
void*X(I*G2,int*G3,int*G4,int*G5,int G6,S*G7){BS G8;G8.A=G2;return Bk(&G8,G3,G4,
G5,G6,G7);}static int W(I*Gx){int Gy;Gy=BR(Gx);R(Gx);return Gy;}static int Bl(BS
*Gw,int*Gx,int*Gy,int*Gz){if(!Bj(Gw,STBI__SCAN_header,0)){R(Gw->A);return 0;}if(
Gx)*Gx=Gw->A->A;if(Gy)*Gy=Gw->A->B;if(Gz)*Gz=Gw->A->C;return 1;}static int Y(I*
G0,int*G1,int*G2,int*G3){BS G4;G4.A=G0;return Bl(&G4,G1,G2,G3);}static int Z(I*
Gx){BS Gy;Gy.A=Gx;if(!Bl(&Gy,0,0,0))return 0;if(Gy.E!=16){R(Gy.A);return 0;}////
return 1;}static int Bm(I*Gw,int*Gx,int*Gy,int*Gz){if(V(Gw,Gx,Gy,Gz))return 1;if
(Y(Gw,Gx,Gy,Gz))return 1;return b("unknown image type");}static int Bn(I*Gw){if(
Z(Gw))return 1;return 0;}extern int stbi_info_from_file(FILE*Gw,int*Gx,int*Gy,//
int*Gz){int G0;I G1;long G2=ftell(Gw);Q(&G1,Gw);G0=Bm(&G1,Gx,Gy,Gz);fseek(Gw,G2,
0);return G0;}extern int stbi_info(char const*Gw,int*Gx,int*Gy,int*Gz){FILE*G0=x
(Gw,"rb");int G1;if(!G0)return b("can't fopen");G1=stbi_info_from_file(G0,Gx,Gy,
Gz);fclose(G0);return G1;}extern int stbi_is_16_bit_from_file(FILE*Gw){int Gx;I
Gy;long Gz=ftell(Gw);Q(&Gy,Gw);Gx=Bn(&Gy);fseek(Gw,Gz,0);return Gx;}extern int//
stbi_is_16_bit(char const*Gw){FILE*Gx=x(Gw,"rb");int Gy;if(!Gx)return b(////////
"can't fopen");Gy=stbi_is_16_bit_from_file(Gx);fclose(Gx);return Gy;}extern int
stbi_info_from_memory(A const*Gw,int Gx,int*Gy,int*Gz,int*G0){I G1;K(&G1,Gw,Gx);
return Bm(&G1,Gy,Gz,G0);}extern int stbi_info_from_callbacks(C const*Gw,void*Gx,
int*Gy,int*Gz,int*G0){I G1;L(&G1,(C*)Gw,Gx);return Bm(&G1,Gy,Gz,G0);}extern int
stbi_is_16_bit_from_memory(A const*Gw,int Gx){I Gy;K(&Gy,Gw,Gx);return Bn(&Gy);}
extern int stbi_is_16_bit_from_callbacks(C const*Gw,void*Gx){I Gy;L(&Gy,(C*)Gw,
Gx);return Bn(&Gy);}typedef uint8_t Bo;typedef uint16_t Bp;typedef uint32_t Bq;
typedef uint64_t Br;typedef enum{STBIR_1CHANNEL=1,STBIR_2CHANNEL=2,STBIR_RGB=3,
STBIR_BGR=0,STBIR_4CHANNEL=5,STBIR_RGBA=4,STBIR_BGRA=6,STBIR_ARGB=7,STBIR_ABGR=8
,STBIR_RA=9,STBIR_AR=10,STBIR_RGBA_PM=11,STBIR_BGRA_PM=12,STBIR_ARGB_PM=13,/////
STBIR_ABGR_PM=14,STBIR_RA_PM=15,STBIR_AR_PM=16,STBIR_RGBA_NO_AW=11,/////////////
STBIR_BGRA_NO_AW=12,STBIR_ARGB_NO_AW=13,STBIR_ABGR_NO_AW=14,STBIR_RA_NO_AW=15,//
STBIR_AR_NO_AW=16,}Bs;typedef enum{STBIR_EDGE_CLAMP=0,STBIR_EDGE_REFLECT=1,/////
STBIR_EDGE_WRAP=2,STBIR_EDGE_ZERO=3,}Bt;typedef enum{STBIR_FILTER_DEFAULT=0,////
STBIR_FILTER_BOX=1,STBIR_FILTER_TRIANGLE=2,STBIR_FILTER_CUBICBSPLINE=3,/////////
STBIR_FILTER_CATMULLROM=4,STBIR_FILTER_MITCHELL=5,STBIR_FILTER_POINT_SAMPLE=6,//
STBIR_FILTER_OTHER=7,}Bu;typedef enum{STBIR_TYPE_UINT8=0,STBIR_TYPE_UINT8_SRGB=1
,STBIR_TYPE_UINT8_SRGB_ALPHA=2,STBIR_TYPE_UINT16=3,STBIR_TYPE_Z1=4,/////////////
STBIR_TYPE_HALF_Z1=5}Bv;typedef void const*Bw(void*optional_output,void const*//
input_ptr,int num_pixels,int x,int y,void*context);typedef void Bx(void const*//
output_ptr,int num_pixels,int y,void*context);typedef Z1 By(Z1 x,Z1 scale,void*
user_data);typedef Z1 Bz(Z1 scale,void*user_data);typedef struct A B0;typedef///
struct B{void*A;void const*B;int C,D;double E,F,G,H;Bw*I;void*J;int K,L;int M,N,
O,P;Bx*Q;int R;int S;int T;int U;int V;int W;Bs X;Bs Y;Bv Z;Bv a;Bu b,c;Bt d,e;
By*f;Bz*g;By*h;Bz*i;B0*j;}B1;typedef enum{STBIRI_1CHANNEL=0,STBIRI_2CHANNEL=1,//
STBIRI_RGB=2,STBIRI_BGR=3,STBIRI_4CHANNEL=4,STBIRI_RGBA=5,STBIRI_BGRA=6,////////
STBIRI_ARGB=7,STBIRI_ABGR=8,STBIRI_RA=9,STBIRI_AR=10,STBIRI_RGBA_PM=11,/////////
STBIRI_BGRA_PM=12,STBIRI_ARGB_PM=13,STBIRI_ABGR_PM=14,STBIRI_RA_PM=15,//////////
STBIRI_AR_PM=16,}B2;static unsigned char B3[]={1,1,1,2,4,2};typedef struct{int A
;int B;}B4;typedef struct{int A;int B;int C;}B5;typedef struct{int A;int B;int C
;}B6;typedef struct C{int A;int B;Z1 C;Z1 D;Z1 E;int F;Bq G,H;}B7;typedef struct
{B4*A;Z1*B;B4*C;Z1*D;B7 E;Z1 F;Bu G;By*H;Bz*I;Bt J;int K;int L;int M;int N;int O
;int P;B5 Q;int R;int S;int T;int U;int V;}B8;typedef struct{B4 A;int B[2];B6 C[
2];}B9;typedef struct{Z1*A;int B;int C;int D;int E,F;int G,H;Z1*I;Z1*J;char K[64
];}B_;typedef Z1*CA(Z1*,int,void const*);typedef void CB(Z1*,int);typedef void//
CC(Z1*,unsigned int,Z1 const*,B4 const*,Z1 const*,int);typedef void CD(Z1*,int);
typedef void CE(void*,int,Z1 const*);struct A{B8 A;B8 B;void const*C;void*D;int
E;int F;int G;int H;Bv I;Bv J;Bw*K;void*L;Bx*M;B9 N;void*O;B_*P;CA*Q;CB*R;CC*S;
CD*T;CE*U;int V;int W;B2 X;B2 Y;int Z;int a,b;int c;int d;int e;size_t f;};/////
static inline int CF(int Gw,int Gx){return Gw<Gx?Gw:Gx;}static inline int CG(int
Gw,int Gx){return Gw>Gx?Gw:Gx;}static Z1 CH[256]={0.0,0.000304f,0.000607f,//////
0.000911f,0.001214f,0.001518f,0.001821f,0.002125f,0.002428f,0.002732f,0.003035f,
0.003347f,0.003677f,0.004025f,0.004391f,0.004777f,0.005182f,0.005605f,0.006049f,
0.006512f,0.006995f,0.007499f,0.008023f,0.008568f,0.009134f,0.009721f,0.010330f,
0.010960f,0.011612f,0.012286f,0.012983f,0.013702f,0.014444f,0.015209f,0.016,////
0.016807f,0.017642f,0.018500f,0.019382f,0.020289f,0.021219f,0.022174f,0.023153f,
0.024158f,0.025187f,0.026241f,0.027321f,0.028426f,0.029557f,0.030713f,0.031896f,
0.033105f,0.034340f,0.035601f,0.036889f,0.038204f,0.039546f,0.040915f,0.042311f,
0.043735f,0.045186f,0.046665f,0.048172f,0.049707f,0.051269f,0.052861f,0.054480f,
0.056128f,0.057805f,0.059511f,0.061246f,0.063010f,0.064803f,0.066626f,0.068478f,
0.070360f,0.072272f,0.074214f,0.076185f,0.078187f,0.080220f,0.082283f,0.084376f,
0.086500f,0.088656f,0.090842f,0.093059f,0.095307f,0.097587f,0.099899f,0.102242f,
0.104616f,0.107023f,0.109462f,0.111932f,0.114435f,0.116971f,0.119538f,0.122139f,
0.124772f,0.127438f,0.130136f,0.132868f,0.135633f,0.138432f,0.141263f,0.144128f,
0.147027f,1.5e-01,0.152926f,0.155926f,0.158961f,0.162029f,0.165132f,0.168269f,//
0.171441f,0.174647f,0.177888f,0.181164f,0.184475f,0.187821f,0.191202f,0.194618f,
0.198069f,0.201556f,0.205079f,0.208637f,0.212231f,0.215861f,0.219526f,0.223228f,
0.226966f,0.230740f,0.234551f,0.238398f,0.242281f,0.246201f,0.250158f,0.254152f,
0.258183f,0.262251f,0.266356f,0.270498f,0.274677f,0.278894f,0.283149f,0.287441f,
0.291771f,0.296138f,0.300544f,0.304987f,0.309469f,0.313989f,0.318547f,0.323143f,
0.327778f,0.332452f,0.337164f,0.341914f,0.346704f,0.351533f,0.356400f,0.361307f,
0.366253f,0.371238f,0.376262f,0.381326f,0.386430f,0.391573f,0.396755f,0.401978f,
0.407240f,0.412543f,0.417885f,0.423268f,0.428691f,0.434154f,0.439657f,0.445201f,
0.450786f,0.456411f,0.462077f,0.467784f,0.473532f,0.479320f,0.485150f,0.491021f,
0.496933f,0.502887f,0.508881f,0.514918f,0.520996f,0.527115f,0.533276f,0.539480f,
0.545725f,0.552011f,0.558340f,0.564712f,0.571125f,0.577581f,0.584078f,0.590619f,
0.597202f,0.603827f,0.610496f,0.617207f,0.623960f,0.630757f,0.637597f,0.644480f,
0.651406f,0.658375f,0.665387f,0.672443f,0.679543f,0.686685f,0.693872f,0.701102f,
0.708376f,0.715694f,0.723055f,0.730461f,0.737911f,0.745404f,0.752942f,0.760525f,
0.768151f,0.775822f,0.783538f,0.791298f,0.799103f,0.806952f,0.814847f,0.822786f,
0.830770f,0.838799f,0.846873f,0.854993f,0.863157f,0.871367f,0.879622f,0.887923f,
0.896269f,0.904661f,0.913099f,0.921582f,0.930111f,0.938686f,0.947307f,0.955974f,
0.964686f,0.973445f,0.982251f,0.991102f,1.0f};typedef union{unsigned int A;Z1 B;
}CI;static const Bq CJ[104]={7536653,7995405,8388621,8847373,9240589,9699341,///
10092557,10551309,10944538,11796506,12648474,13500442,14286874,15138842,15990810
,16842778,17694771,19398707,21037107,22741043,24444979,26148915,27787315,///////
29491251,31195239,34537575,37945447,41287783,44695655,48037991,51445863,54788199
,58196174,64946382,71696590,78446798,85197006,91947205,98369724,0x063b00b5,/////
0x06970158,0x07420142,0x07e30130,0x087b0120,0x090b0112,0x09940106,0x0a1700fc,///
0x0a9500f2,0x0b0f01cb,0x0bf401ae,0x0ccb0195,0x0d950180,0x0e56016e,0x0f0d015e,///
0x0fbc0150,0x10630143,0x11070264,0x1238023e,0x1357021d,0x14660201,0x156601e9,///
0x165a01d3,0x174401c0,0x182401af,0x18fe0331,0x1a9602fe,0x1c1502d2,0x1d7e02ad,///
0x1ed4028d,0x201a0270,0x21520256,0x227d0240,0x239f0443,0x25c003fe,0x27bf03c4,///
0x29a10392,0x2b6a0367,0x2d1d0341,0x2ebe031f,0x304d0300,0x31d105b0,0x34a80555,///
0x37520507,0x39d504c5,0x3c37048b,0x3e7c0458,0x40a8042a,0x42bd0401,0x44c20798,///
0x488e071e,0x4c1c06b6,0x4f76065d,0x52a50610,0x55ac05cc,0x5892058f,0x5b590559,///
0x5e0c0a23,0x631c0980,0x67db08f6,0x6c55087f,0x70940818,0x74a007bd,0x787d076c,///
0x7c330723,};static inline Bo CK(Z1 Gw){static const CI Gx={0x3f7fffff};static//
const CI Gy={956301312};Bq Gz,G0,G1,G2;CI G3;if(!(Gw>Gy.B))return 0;if(Gw>Gx.B)
return 255;G3.B=Gw;Gz=CJ[(G3.A-Gy.A)>>20];G0=(Gz>>16)<<9;G1=Gz&0xffff;G2=(G3.A>>
12)&0xff;return(unsigned char)((G0+G1*G2)>>16);}typedef union D{unsigned short A
;}CL;static inline Z1 CM(CL Gw){static const CI Gx={2004877312};static const CI
Gy={1199570944};CI Gz;Gz.A=(Gw.A&0x7fff)<<13;Gz.B*=Gx.B;if(Gz.B>=Gy.B)Gz.A|=255
<<23;Gz.A|=(Gw.A&0x8000)<<16;return Gz.B;}static inline CL CN(Z1 Gw){CI Gx={255
<<23};CI Gy={1199570944};CI Gz={1056964608};unsigned int G0=0x80000000u;CL G1={0
};CI G2;unsigned int G3;G2.B=Gw;G3=G2.A&G0;G2.A^=G3;if(G2.A>=Gy.A)G1.A=(G2.A>Gx.
A)?0x7e00:0x7c00;else if(G2.A<947912704){G2.B+=Gz.B;G1.A=(unsigned short)(G2.A-
Gz.A);}else{unsigned int G4=(G2.A>>13)&1;G2.A=G2.A+3355443200u+0xfff;G2.A+=G4;G1
.A=(unsigned short)(G2.A>>13);}G1.A|=G3>>16;return G1;}static void CO(void*Gw,//
void const*Gx,size_t Gy){char*RESTRICT Gz=(char*)Gx;char*RESTRICT G0=((char*)Gx)
+Gy;ptrdiff_t G1=(char*)Gw-(char*)Gx;if(G1>=8){char*RESTRICT G2=((char*)Gx)+(Gy&
~7);if(((((ptrdiff_t)Gw)|((ptrdiff_t)Gx))&7)==0)do{*(Br*)(Gz+G1)=*(Br*)Gz;Gz+=8;
}while(Gz<G2);else do{int G3,G4;G3=((int*)Gz)[0];G4=((int*)Gz)[1];((int*)(Gz+G1)
)[0]=G3;((int*)(Gz+G1))[1]=G4;Gz+=8;}while(Gz<G2);if(Gz==G0)return;}do{*(int*)(
Gz+G1)=*(int*)Gz;Gz+=4;}while(Gz<G0);}static Z1 CP(Z1 Gw,Z1 Gx,void*Gy){Z1 Gz=Gx
/2;Z1 G0=0.5f+Gz;if(Gw<0.0f)Gw=-Gw;if(Gw>=G0)return 0.0f;else{Z1 G1=0.5f-Gz;if(
Gw<=G1)return 1.0f;else return(G0-Gw)/Gx;}}static Z1 CQ(Z1 Gw,void*Gx){return///
0.5f+Gw/2.0f;}static Z1 CR(Z1 Gw,Z1 Gx,void*Gy){if(Gw<0.0f)Gw=-Gw;if(Gw<=1.0f)//
return 1.0f-Gw;else return 0.0f;}static Z1 CS(Z1 Gw,Z1 Gx,void*Gy){return 1.0f;}
static Z1 CT(Z1 Gw,Z1 Gx,void*Gy){if(Gw<0.0f)Gw=-Gw;if(Gw<1.0f)return(4.0f+Gw*Gw
*(3.0f*Gw-6.0f))/6.0f;else if(Gw<2.0f)return(8.0f+Gw*(-12.0+Gw*(6.0f-Gw)))/6.0f;
return 0.0f;}static Z1 CU(Z1 Gw,Z1 Gx,void*Gy){if(Gw<0.0f)Gw=-Gw;if(Gw<1.0f)////
return 1.0f-Gw*Gw*(2.5f-1.5f*Gw);else if(Gw<2.0f)return 2.0f-Gw*(4.0f+Gw*(0.5f*
Gw-2.5f));return 0.0f;}static Z1 CV(Z1 Gw,Z1 Gx,void*Gy){if(Gw<0.0f)Gw=-Gw;if(Gw
<1.0f)return(16.0f+Gw*Gw*(21.0f*Gw-36.0f))/18.0f;else if(Gw<2.0f)return(32.0f+Gw
*(-60.0+Gw*(36.0f-7.0f*Gw)))/18.0f;return 0.0f;}static Z1 CW(Z1 Gw,void*Gx){////
return 0.5f;}static Z1 CX(Z1 Gw,void*Gx){return 1;}static Z1 CZ(Z1 Gw,void*Gx){
return 2;}static int Ca(Bz*Gw,Z1 Gx,void*Gy){if(Gx>=1.0)return(int)((Z1)ceil((Z1
)(Gw(1.0f/Gx,Gy)*2.0f)));else return(int)((Z1)ceil((Z1)(Gw(Gx,Gy)*2.0f/Gx)));}//
static int Cb(B8*Gw,int Gx,void*Gy){Z1 Gz=Gw->E.C;Bz*G0=Gw->I;switch(Gx){case 1:
return(int)((Z1)ceil((Z1)(G0(1.0f/Gz,Gy)*2.0f)));case 2:return(int)((Z1)ceil((Z1
)(G0(Gz,Gy)*2.0f/Gz)));case 0:return(int)((Z1)ceil((Z1)(G0(Gz,Gy)*2.0f)));//////
default:return 0;}}static int Cc(B8*Gw,int Gx){if(Gx)return Gw->E.B;else return
Gw->E.A+Gw->M*2;}static int Cd(int Gw,int Gx){return 0;}static int Ce(int Gw,int
Gx){if(Gw<0)return 0;if(Gw>=Gx)return Gx-1;return Gw;}static int Cf(int Gw,int//
Gx){if(Gw<0)if(Gw>-Gx)return-Gw;else return Gx-1;if(Gw>=Gx){int Gy=Gx*2;if(Gw>=
Gy)return 0;else return Gy-Gw-1;}return Gw;}static int Cg(int Gw,int Gx){if(Gw>=
0)return Gw%Gx;else{int Gy=(-Gw)%Gx;if(Gy!=0)Gy=Gx-Gy;return Gy;}}typedef int Ch
(int n,int max);static Ch*Ci[]={Ce,Cf,Cg,Cd,};inline static int Cj(Bt Gw,int Gx,
int Gy){if(Gx>=0&&Gx<Gy)return Gx;return Ci[Gw](Gx,Gy);}static void Ck(B8*Gw,B9*
Gx){int Gy,Gz;int G0,G1;int G2=0x7fffffff,G3=-2147483647;int G4=0x7fffffff,G5=-
0x7fffffff;int G6=0x7fffffff,G7=-0x7fffffff;Bt G8=Gw->J;B4*G9=Gw->A;int G_=Gw->E
.B;int HA=Gw->E.A;int HB=Gw->M;Gz=G_;for(Gy=0;Gy<Gz;Gy++)if(G9[Gy].A<G2){G2=G9[
Gy].A;Gz=Gy+HB;if(Gz>G_)Gz=G_;}Gz=0;for(Gy=G_-1;Gy>=Gz;Gy--)if(G9[Gy].B>G3){G3=
G9[Gy].B;Gz=Gy-HB;if(Gz<0)Gz=0;}G0=0;if(G2<0){G0=-G2;G2=0;}G1=0;if(G3>=HA){G1=G3
-HA+1;G3=HA-1;}Gx->B[0]=G0;Gx->B[1]=G1;Gx->C[0].A=G2;Gx->C[0].B=G3;Gx->C[0].C=G2
;Gx->C[1].A=0;Gx->C[1].B=-1;Gx->C[1].C=0;if(G8==STBIR_EDGE_ZERO)return;for(Gy= -
G0;Gy<0;Gy++){int HC=Cj(G8,Gy,HA);if(HC<G4)G4=HC;if(HC>G5)G5=HC;}for(Gy=HA;Gy<(
HA+G1);Gy++){int HC=Cj(G8,Gy,HA);if(HC<G6)G6=HC;if(HC>G7)G7=HC;}if(G4!=/////////
0x7fffffff)if(((G4<=G2)&&((G5+16)>=G2))||((G2<=G4)&&((G3+16)>=G5))){Gx->C[0].A=
G2=CF(G2,G4);Gx->C[0].B=G3=CG(G3,G5);Gx->C[0].C=G2;G0=0;}if(G6!=0x7fffffff)if(((
G6<=G2)&&((G7+16)>=G2))||((G2<=G6)&&((G3+16)>=G7))){Gx->C[0].A=G2=CF(G2,G6);Gx->
C[0].B=G3=CG(G3,G7);Gx->C[0].C=G2;G1=0;}if(G0&&(G4!=0x7fffffff)){B6*HC=Gx->C+1;
if(G4<Gx->C[0].A){Gx->C[1].C=Gx->C[0].A;Gx->C[1].A=Gx->C[0].A;Gx->C[1].B=Gx->C[0
].B;--HC;}HC->C=G4;HC->A=-G0;HC->B=(G5-G4)-G0;Gx->B[0]=0;}else if(G1&&(G6!=/////
0x7fffffff)){B6*HC=Gx->C+1;if(G6<Gx->C[0].A){Gx->C[1].C=Gx->C[0].A;Gx->C[1].A=Gx
->C[0].A;Gx->C[1].B=Gx->C[0].B;--HC;}HC->C=G6;HC->A=Gx->C[1].B+1;HC->B=Gx->C[1].
B+1+(G7-G6);Gx->B[1]=0;}if((Gx->C[1].B>Gx->C[1].A)&&(Gx->C[0].A>Gx->C[1].A)){B6
HC=Gx->C[0];Gx->C[0]=Gx->C[1];Gx->C[1]=HC;}}static void Cl(int*Gw,int*Gx,Z1 Gy,
Z1 Gz,Z1 G0,Z1 G1,int G2,Bt G3){int G4,G5;Z1 G6=Gy-Gz;Z1 G7=Gy+Gz;Z1 G8=(G6+G1)*
G0;Z1 G9=(G7+G1)*G0;G4=(int)(Z1)floor((Z1)(G8+0.5f));G5=(int)(Z1)floor((Z1)(G9-
0.5f));if(G5<G4)G5=G4;if(G3==STBIR_EDGE_WRAP){if(G4<-G2)G4=-G2;if(G5>=(G2*2))G5=
(G2*2)-1;}*Gw=G4;*Gx=G5;}static void Cm(Z1 Gw,By*Gx,B7*Gy,int Gz,B4*G0,Z1*G1,int
G2,Bt G3,void*G4){int G5,G6;Z1 G7=Gy->D;Z1 G8=Gy->E;int G9=Gy->A;int G_=Gy->G;//
int HA=Gy->F&&(G_<Gz);G6=Gz;if(HA)G6=G_;for(G5=0;G5<G6;G5++){int HB;int HC;Z1 HD
=(Z1)G5+0.5f;Z1 HE=(HD+G8)*G7;int HF,HG;Cl(&HF,&HG,HD,Gw,G7,G8,G9,G3);if((HG-HF+
1)>G2)HG=HF+G2-1;HC=-1;for(HB=0;HB<=HG-HF;HB++){Z1 HH=(Z1)(HB+HF)+0.5f;Z1 HI=Gx(
HE-HH,G7,G4);if((HI<7.523e-37)&&(HI>-7.523e-37)){if(HB==0){++HF;HB--;continue;}
HI=0;}else HC=HB;G1[HB]=HI;}HG=HC+HF;G0->A=HF;G0->B=HG;++G0;G1+=G2;}}static void
Cn(B4*Gw,Z1*Gx,int Gy,Z1 Gz,int G0){if(Gw->B<Gw->A){Gw->A=Gw->B=Gy;Gx[0]=Gz;}///
else if(Gy<=Gw->B){if(Gy<Gw->A){if((Gw->B-Gy+1)<=G0){int G1,G2=Gw->A-Gy;for(G1=
Gw->B-Gw->A;G1>=0;G1--)Gx[G1+G2]=Gx[G1];for(G1=1;G1<G2;G1++)Gx[G1]=0;Gx[0]=Gz;Gw
->A=Gy;}}else Gx[Gy-Gw->A]+=Gz;}else if((Gy-Gw->A+1)<=G0){int G1,G2=Gy-Gw->A;for
(G1=(Gw->B-Gw->A)+1;G1<G2;G1++)Gx[G1]=0;Gx[G2]=Gz;Gw->B=Gy;}}static void Co(int*
Gw,int*Gx,Z1 Gy,Z1 Gz,Z1 G0,Z1 G1,int G2){Z1 G3=Gy-Gz;Z1 G4=Gy+Gz;Z1 G5=G3*G0-G1
;Z1 G6=G4*G0-G1;int G7=(int)(Z1)floor((Z1)(G5+0.5f));int G8=(int)(Z1)floor((Z1)(
G6-0.5f));if(G7<0)G7=0;if(G8>=G2)G8=G2-1;*Gw=G7;*Gx=G8;}static void Cp(int Gw,//
int Gx,Z1 Gy,By*Gz,B7*G0,int G1,int G2,B4*G3,Z1*G4,void*G5){int G6;int G7;int G8
=-1;Z1 G9=G0->C;Z1 G_=G0->E;int HA=G0->B;int HB=G0->G;int HC=G0->F&&(HB<HA);for(
G6=Gw;G6<Gx;G6++){Z1 HD=(Z1)G6+0.5f;Z1 HE=HD*G9-G_;int HF,HG;Co(&HF,&HG,HD,Gy,G9
,G_,HA);if(HF>HG)continue;if(HC){if(HF==HB)break;if(HG>=HB)HG=HB-1;}for(G7=0;G7
<=HG-HF;G7++){Z1 HH=(Z1)(G7+HF)+0.5f;Z1 HI=HH-HE;Z1 HJ=Gz(HI,G9,G5)*G9;if((HJ<//
7.523e-37)&&(HJ>-7.523e-37))HJ=0.0f;{int HK=G7+HF;Z1*HL=G4+HK*G1;B4*HM=G3+HK;if(
HK>G8){G8=HK;HM->A=G6;HM->B=G6;HL[0]=HJ;}else{if(HL[0]==0.0f)HM->A=G6;HM->B=G6;
HL[G6-HM->A]=HJ;}}}}}static void Cq(Bt Gw,B5*Gx,B7*Gy,int Gz,B4*G0,Z1*G1,int G2)
{int G3=Gy->A;int G4=G3-1;int G5,G6;int G7=0x7fffffff;int G8=-2147483647;int G9=
-1;int G_=Gy->G;int HA=Gy->H;int HB=Gy->F&&(G_<Gz);Z1*HC;B4*HD;HC=G1;HD=G0;G6=Gz
;if(HB)G6=G_;for(G5=0;G5<G6;G5++){int HE;double HF,HG=0;int HH;HH=HD->B-HD->A;//
for(HE=0;HE<=HH;HE++)HG+=(double)HC[HE];if((HG<7.523e-37)&&(HG>-7.523e-37)){HD->
B=HD->A;HC[0]=0.0f;}else if((HG<1.0)||(HG>1.0)){HF=1.0/HG;for(HE=0;HE<=HH;HE++)
HC[HE]=(Z1)(HC[HE]*HF);}++HD;HC+=G2;}if(HB){B4*HE=G0;B4*HF=G0+G_;for(G5=G_;G5<Gz
;G5++){HF->A=HE->A+HA;HF->B=HE->B+HA;++HF;++HE;}CO(G1+G_*G2,G1,(Gz-G_)*G2*4);}HC
=G1;HD=G0;for(G5=0;G5<Gz;G5++){int HE;if(Gw==STBIR_EDGE_ZERO){if(HD->B>G4)HD->B=
G4;if(HD->A<0){int HF,HG,HH=0;HH=-HD->A;HD->A=0;HG=HD->B-HD->A+1;if(HG>0)for(HF=
0;HF<HG;HF++)HC[HF]=HC[HF+HH];}}else if((Gw==STBIR_EDGE_CLAMP)||(Gw==///////////
STBIR_EDGE_REFLECT)){if(HD->B>G4){int HF=HD->A;int HG=HD->B;HD->B=G4;for(HE=G3;
HE<=HG;HE++)Cn(HD,HC,Ci[Gw](HE,G3),HC[HE-HF],G2);}if(HD->A<0){int HF;Z1 HG;Z1*HH
=HC-(HD->A+1);for(HE=-1;HE>HD->A;HE--)Cn(HD,HC,Ci[Gw](HE,G3),*HH--,G2);HF=HD->A;
HG=HH[0];HD->A=0;for(HE=0;HE<=HD->B;HE++)HC[HE]=HC[HE-HF];Cn(HD,HC,Ci[Gw](HF,G3)
,HG,G2);}}if(HD->A<=HD->B){int HF=HD->B-HD->A+1;while(HF&&(HC[HF-1]==0.0f))--HF;
HD->B=HD->A+HF-1;if(HD->A<=HD->B){if(HD->A<G7)G7=HD->A;if(HD->B>G8)G8=HD->B;if(
HF>G9)G9=HF;}for(HE=HF;HE<G2;HE++)HC[HE]=0.0f;}++HD;HC+=G2;}Gx->A=G7;Gx->B=G8;Gx
->C=G9;}static int Cr(int Gw,B4*Gx,Z1*Gy,int Gz,int G0,int G1,int G2){int G3=G2+
1;if(Gz!=G0){Z1*G4=Gy;Z1*G5=Gy;Z1*G6=Gy+Gw*G0;switch(G0){case 1:do{{((Bq*)G4)[0]
=((Bq*)G5)[0];}++G4;G5+=Gz;}while(G4<G6);break;case 2:do{{((Br*)G4)[0]=((Br*)G5)
[0];}G4+=2;G5+=Gz;}while(G4<G6);break;case 3:do{{((Br*)G4)[0]=((Br*)G5)[0];}{((
Bq*)(G4+2))[0]=((Bq*)(G5+2))[0];}G4+=3;G5+=Gz;}while(G4<G6);break;case 4:do{{((
Br*)G4)[0]=((Br*)G5)[0];((Br*)G4)[1]=((Br*)G5)[1];}G4+=4;G5+=Gz;}while(G4<G6);//
break;case 5:do{{((Br*)G4)[0]=((Br*)G5)[0];((Br*)G4)[1]=((Br*)G5)[1];}{((Bq*)(G4
+4))[0]=((Bq*)(G5+4))[0];}G4+=5;G5+=Gz;}while(G4<G6);break;case 6:do{{((Br*)G4)[
0]=((Br*)G5)[0];((Br*)G4)[1]=((Br*)G5)[1];}{((Br*)(G4+4))[0]=((Br*)(G5+4))[0];}
G4+=6;G5+=Gz;}while(G4<G6);break;case 7:do{{((Br*)G4)[0]=((Br*)G5)[0];((Br*)G4)[
1]=((Br*)G5)[1];}{((Br*)(G4+4))[0]=((Br*)(G5+4))[0];}{((Bq*)(G4+6))[0]=((Bq*)(G5
+6))[0];}G4+=7;G5+=Gz;}while(G4<G6);break;case 8:do{{((Br*)G4)[0]=((Br*)G5)[0];(
(Br*)G4)[1]=((Br*)G5)[1];}{((Br*)(G4+4))[0]=((Br*)(G5+4))[0];((Br*)(G4+4))[1]=((
Br*)(G5+4))[1];}G4+=8;G5+=Gz;}while(G4<G6);break;case 9:do{{((Br*)G4)[0]=((Br*)
G5)[0];((Br*)G4)[1]=((Br*)G5)[1];}{((Br*)(G4+4))[0]=((Br*)(G5+4))[0];((Br*)(G4+4
))[1]=((Br*)(G5+4))[1];}{((Bq*)(G4+8))[0]=((Bq*)(G5+8))[0];}G4+=9;G5+=Gz;}while(
G4<G6);break;case 10:do{{((Br*)G4)[0]=((Br*)G5)[0];((Br*)G4)[1]=((Br*)G5)[1];}{(
(Br*)(G4+4))[0]=((Br*)(G5+4))[0];((Br*)(G4+4))[1]=((Br*)(G5+4))[1];}{((Br*)(G4+8
))[0]=((Br*)(G5+8))[0];}G4+=10;G5+=Gz;}while(G4<G6);break;case 11:do{{((Br*)G4)[
0]=((Br*)G5)[0];((Br*)G4)[1]=((Br*)G5)[1];}{((Br*)(G4+4))[0]=((Br*)(G5+4))[0];((
Br*)(G4+4))[1]=((Br*)(G5+4))[1];}{((Br*)(G4+8))[0]=((Br*)(G5+8))[0];}{((Bq*)(G4+
10))[0]=((Bq*)(G5+10))[0];}G4+=11;G5+=Gz;}while(G4<G6);break;case 12:do{{((Br*)
G4)[0]=((Br*)G5)[0];((Br*)G4)[1]=((Br*)G5)[1];}{((Br*)(G4+4))[0]=((Br*)(G5+4))[0
];((Br*)(G4+4))[1]=((Br*)(G5+4))[1];}{((Br*)(G4+8))[0]=((Br*)(G5+8))[0];((Br*)(
G4+8))[1]=((Br*)(G5+8))[1];}G4+=12;G5+=Gz;}while(G4<G6);break;default:do{Z1*G7=
G4+G0-4;Z1*G8=G5;do{{((Br*)G4)[0]=((Br*)G8)[0];((Br*)G4)[1]=((Br*)G8)[1];}G4+=4;
G8+=4;}while(G4<=G7);G7+=4;while(G4<G7){{((Bq*)G4)[0]=((Bq*)G8)[0];}++G4;++G8;}
G5+=Gz;}while(G4<G6);break;}}Gy[G0*Gw]=8888.0f;{B4*G4=Gx+Gw-1;Z1*G5=Gy+G0*(Gw-1)
;while((G4>=Gx)&&((G4->A+G0*2)>=G3)){if((G4->A+G0)>G3){int G6=G0;if(G0>12){int//
G7;G7=G0&3;G6=(((G4->B-G4->A+1)-G7+3)&~3)+G7;if(G6<(8+G7))G6=8+G7;}if((G4->A+G6)
>G3){int G7=G3-G6;int G8=G4->B-G4->A+1;int G9=G4->A-G7;Z1*G_=G5+G8-1;Z1*HA=G_+G9
;while(G8){*HA-- =*G_--;--G8;}while(HA>=G5)*HA-- =0;G4->A=G7;if(G0>12){int HB;HB
=G0&3;G6=(((G4->B-G4->A+1)-HB+3)&~3)+HB;if(G6<(8+HB))G6=8+HB;}}}--G4;G5-=G0;}}//
return G0;}static void Cs(B8*Gw,B8*Gx,void*Gy){int Gz;Z1 G0=Gw->E.C;By*G1=Gw->H;
Bz*G2=Gw->I;Z1 G3=Gw->E.D;int G4=Gw->E.A;int G5=Gw->N;B4*G6=Gw->A;Z1*G7=Gw->B;//
int G8=Gw->K;switch(Gw->R){case 1:{Z1 G9=G2(G3,Gy)*G0;Cm(G9,G1,&Gw->E,G5,G6,G7,
G8,Gw->J,Gy);Cq(Gw->J,&Gw->Q,&Gw->E,G5,G6,G7,G8);}break;case 0:case 2:{Z1 G9=G2(
G0,Gy)*G3;int G_=Gw->M;int HA=G4+G_;if(!Gw->R){if(Gx){G6=Gx->A;G7=Gx->B;G8=Gx->K
;G5=Gx->N;Gw->Q.A=Gx->Q.A;Gw->Q.B=Gx->Q.B;Gw->Q.C=Gx->Q.C;goto//////////////////
jump_right_to_pivot;}G6=Gw->C;G7=Gw->D;G8=Gw->T;G5=Gw->S;}Cp(-G_,HA,G9,G1,&Gw->E
,G8,G5,G6,G7,Gy);Cq(Gw->J,&Gw->Q,&Gw->E,G5,G6,G7,G8);if(!Gw->R){B4*HB;int HC;///
jump_right_to_pivot:HC=(-G_)-1;for(Gz=0;Gz<G5;Gz++){int HD;int HE=G6->A,HF=G6->B
;int HG=Gw->K;Z1*HH=Gw->B+(HE+G_)*HG;Z1*HI=G7;HB=Gw->A+(HE+G_);for(HD=HE;HD<=HF;
HD++){Z1 HJ=*HI++;if((HJ>=7.523e-37)||(HJ<=-7.523e-37))if((HD>HC)||(HB->A>HB->B)
){{B4*HK=Gw->A+(HC+G_+1);while(HK<HB){HK->A=0;HK->B=-1;++HK;}}HB->A=Gz;HB->B=Gz;
HH[0]=HJ;HC=HD;}else Cn(HB,HH,Gz,HJ,HG);++HB;HH+=HG;}++G6;G7+=G8;}{B4*HD=Gw->A+(
HC+G_+1);B4*HE=Gw->A+Gw->N;while(HD<HE){HD->A=0;HD->B=-1;++HD;}}}}break;}}static
Z1*Ct Z4((Z1)G1[0])*3.922e-03;Gz[-3]=((Z1)G1[1])*3.922e-03;Gz[-2]=((Z1)G1[2])*//
3.922e-03;Gz[-1]=((Z1)G1[3])*3.922e-03;Gz+=4;G1+=4;}Gz-=4;while(Gz<G0){Gz[0]=((
Z1)G1[0])*3.922e-03;Gz+=1;G1+=1;}return G0;}static void Cu Zd Gz+=4;while(Gz<=G0
){Z1 G1;G1=Gy[0]*255.0f Z2 255)G1=255;break;}Gz[-4]=(unsigned char)G1;G1=Gy[1]*
255.0f Z2 255)G1=255;break;}Gz[-3]=(unsigned char)G1;G1=Gy[2]*255.0f Z2 255)G1=
255;break;}Gz[-2]=(unsigned char)G1;G1=Gy[3]*255.0f Z2 255)G1=255;break;}Gz[-1]=
(unsigned char)G1;Gz+=4;Gy+=4;}Gz-=4;while(Gz<G0){Z1 G1;G1=Gy[0]*255.0f Z2 255)
G1=255;break;}Gz[0]=(unsigned char)G1;Gz+=1;Gy+=1;}}static Z1*Cv Z4((Z1)G1[0]);
Gz[-3]=((Z1)G1[1]);Gz[-2]=((Z1)G1[2]);Gz[-1]=((Z1)G1[3]);Gz+=4;G1+=4;}Gz-=4;////
while(Gz<G0){Gz[0]=((Z1)G1[0]);Gz+=1;G1+=1;}return G0;}static void Cw Zd Gz+=4;
while(Gz<=G0){Z1 G1;G1=Gy[0]Z2 255)G1=255;break;}Gz[-4]=(unsigned char)G1;G1=Gy[
1]Z2 255)G1=255;break;}Gz[-3]=(unsigned char)G1;G1=Gy[2]Z2 255)G1=255;break;}Gz[
-2]=(unsigned char)G1;G1=Gy[3]Z2 255)G1=255;break;}Gz[-1]=(unsigned char)G1;Gz+=
4;Gy+=4;}Gz-=4;while(Gz<G0){Z1 G1;G1=Gy[0]Z2 255)G1=255;break;}Gz[0]=(unsigned//
char)G1;Gz+=1;Gy+=1;}}static Z1*Cx Z4 CH[G1[0]];Gz[-3]=CH[G1[1]];Gz[-2]=CH[G1[2]
];Gz[-1]=CH[G1[3]];Gz+=4;G1+=4;}Gz-=4;while(Gz<G0){Gz[0]=CH[G1[0]];Gz+=1;G1+=1;}
return G0;}static void Cy Zd Gz+=4;while(Gz<=G0){Gz[-4]=CK(Gy[0]);Gz[-3]=CK(Gy[1
]);Gz[-2]=CK(Gy[2]);Gz[-1]=CK(Gy[3]);Gz+=4;Gy+=4;}Gz-=4;while(Gz<G0){Gz[0]=CK(Gy
[0]);Gz+=1;Gy+=1;}}static Z1*Cz Z3 do{Gz[0]=CH[G1[0]];Gz[1]=CH[G1[1]];Gz[2]=CH[
G1[2]];Gz[3]=((Z1)G1[3])*3.922e-03;G1+=4;Gz+=4;}while(Gz<G0);return G0;}static//
void C0 Zd do{Z1 G1;Gz[0]=CK(Gy[0]);Gz[1]=CK(Gy[1]);Gz[2]=CK(Gy[2]);G1=Gy[3]*///
255.0f Z2 255)G1=255;break;}Gz[3]=(unsigned char)G1;Gz+=4;Gy+=4;}while(Gz<G0);}
static Z1*C1 Z4 CH[G1[0]];Gz[-3]=((Z1)G1[1])*3.922e-03;Gz[-2]=CH[G1[2]];Gz[-1]=(
(Z1)G1[3])*3.922e-03;G1+=4;Gz+=4;}Gz-=4;if(Gz<G0){Gz[0]=CH[G1[0]];Gz[1]=((Z1)G1[
1])*3.922e-03;}return G0;}static void C2 Zd do{Z1 G1;Gz[0]=CK(Gy[0]);G1=Gy[1]*//
255.0f Z2 255)G1=255;break;}Gz[1]=(unsigned char)G1;Gz+=2;Gy+=2;}while(Gz<G0);}
static Z1*C3 Zj 0])*1.526e-05;Gz[-3]=((Z1)G1[1])*1.526e-05;Gz[-2]=((Z1)G1[2])*//
1.526e-05;Gz[-1]=((Z1)G1[3])*1.526e-05;Gz+=4;G1+=4;}Gz-=4;while(Gz<G0){Gz[0]=((
Z1)G1[0])*1.526e-05;Gz+=1;G1+=1;}return G0;}static void C4 Zi 0]*65535.0f Z2////
65535)G1=65535;break;}Gz[-4]=(unsigned short)G1;G1=Gy[1]*65535.0f Z2 65535)G1=//
65535;break;}Gz[-3]=(unsigned short)G1;G1=Gy[2]*65535.0f Z2 65535)G1=65535;break
;}Gz[-2]=(unsigned short)G1;G1=Gy[3]*65535.0f Z2 65535)G1=65535;break;}Gz[-1]=(
unsigned short)G1;Gz+=4;Gy+=4;}Gz-=4;while(Gz<G0){Z1 G1;G1=Gy[0]*65535.0f Z2////
65535)G1=65535;break;}Gz[0]=(unsigned short)G1;Gz+=1;Gy+=1;}}static Z1*C5 Zj 0])
;Gz[-3]=((Z1)G1[1]);Gz[-2]=((Z1)G1[2]);Gz[-1]=((Z1)G1[3]);Gz+=4;G1+=4;}Gz-=4;///
while(Gz<G0){Gz[0]=((Z1)G1[0]);Gz+=1;G1+=1;}return G0;}static void C6 Zi 0]Z2///
65535)G1=65535;break;}Gz[-4]=(unsigned short)G1;G1=Gy[1]Z2 65535)G1=65535;break;
}Gz[-3]=(unsigned short)G1;G1=Gy[2]Z2 65535)G1=65535;break;}Gz[-2]=(unsigned////
short)G1;G1=Gy[3]Z2 65535)G1=65535;break;}Gz[-1]=(unsigned short)G1;Gz+=4;Gy+=4;
}Gz-=4;while(Gz<G0){Z1 G1;G1=Gy[0]Z2 65535)G1=65535;break;}Gz[0]=(unsigned short
)G1;Gz+=1;Gy+=1;}}static Z1*C7(Z1*Gw,int Gx,void const*Gy){Z1*RESTRICT Gz=Gw;Z1*
G0=(Z1*)Gz+Gx;CL const*G1=(CL const*)Gy;Gz+=4;while(Gz<=G0){Gz[-4]=CM(G1[0]);Gz[
-3]=CM(G1[1]);Gz[-2]=CM(G1[2]);Gz[-1]=CM(G1[3]);Gz+=4;G1+=4;}Gz-=4;while(Gz<G0){
Gz[0]=CM(G1[0]);Gz+=1;G1+=1;}return G0;}static void C8(void*Gw,int Gx,Z1 const*
Gy){CL*RESTRICT Gz=(CL*)Gw;CL*G0=((CL*)Gz)+Gx;Gz+=4;while(Gz<=G0){Gz[-4]=CN(Gy[0
]);Gz[-3]=CN(Gy[1]);Gz[-2]=CN(Gy[2]);Gz[-1]=CN(Gy[3]);Gz+=4;Gy+=4;}Gz-=4;while(
Gz<G0){Gz[0]=CN(Gy[0]);Gz+=1;Gy+=1;}}static Z1*C9(Z1*Gw,int Gx,void const*Gy){if
((void*)Gw!=Gy)memcpy(Gw,Gy,Gx*4);return Gw+Gx;}static void C_(void*Gw,int Gx,Z1
const*Gy){if((void*)Gw!=(void*)Gy)memcpy(Gw,Gy,Gx*4);}static Z1*DA Z4((Z1)G1[2])
*3.922e-03;Gz[-3]=((Z1)G1[1])*3.922e-03;Gz[-2]=((Z1)G1[0])*3.922e-03;Gz[-1]=((Z1
)G1[3])*3.922e-03;Gz+=4;G1+=4;}Gz-=4;return G0;}static void DB Zd Gz+=4;while(Gz
<=G0){Z1 G1;G1=Gy[2]*255.0f Z2 255)G1=255;break;}Gz[-4]=(unsigned char)G1;G1=Gy[
1]*255.0f Z2 255)G1=255;break;}Gz[-3]=(unsigned char)G1;G1=Gy[0]*255.0f Z2 255)
G1=255;break;}Gz[-2]=(unsigned char)G1;G1=Gy[3]*255.0f Z2 255)G1=255;break;}Gz[-
1]=(unsigned char)G1;Gz+=4;Gy+=4;}Gz-=4;}static Z1*DC Z4((Z1)G1[2]);Gz[-3]=((Z1)
G1[1]);Gz[-2]=((Z1)G1[0]);Gz[-1]=((Z1)G1[3]);Gz+=4;G1+=4;}Gz-=4;return G0;}/////
static void DD Zd Gz+=4;while(Gz<=G0){Z1 G1;G1=Gy[2]Z2 255)G1=255;break;}Gz[-4]=
(unsigned char)G1;G1=Gy[1]Z2 255)G1=255;break;}Gz[-3]=(unsigned char)G1;G1=Gy[0]
Z2 255)G1=255;break;}Gz[-2]=(unsigned char)G1;G1=Gy[3]Z2 255)G1=255;break;}Gz[-1
]=(unsigned char)G1;Gz+=4;Gy+=4;}Gz-=4;}static Z1*DE Z4 CH[G1[2]];Gz[-3]=CH[G1[1
]];Gz[-2]=CH[G1[0]];Gz[-1]=CH[G1[3]];Gz+=4;G1+=4;}Gz-=4;return G0;}static void//
DF Zd Gz+=4;while(Gz<=G0){Gz[-4]=CK(Gy[2]);Gz[-3]=CK(Gy[1]);Gz[-2]=CK(Gy[0]);Gz[
-1]=CK(Gy[3]);Gz+=4;Gy+=4;}Gz-=4;}static Z1*DG Z3 do{Gz[0]=CH[G1[2]];Gz[1]=CH[G1
[1]];Gz[2]=CH[G1[0]];Gz[3]=((Z1)G1[3])*3.922e-03;G1+=4;Gz+=4;}while(Gz<G0);/////
return G0;}static void DH Zd do{Z1 G1;Gz[2]=CK(Gy[0]);Gz[1]=CK(Gy[1]);Gz[0]=CK(
Gy[2]);G1=Gy[3]*255.0f Z2 255)G1=255;break;}Gz[3]=(unsigned char)G1;Gz+=4;Gy+=4;
}while(Gz<G0);}static Z1*DI Zj 2])*1.526e-05;Gz[-3]=((Z1)G1[1])*1.526e-05;Gz[-2]
=((Z1)G1[0])*1.526e-05;Gz[-1]=((Z1)G1[3])*1.526e-05;Gz+=4;G1+=4;}Gz-=4;return G0
;}static void DJ Zi 2]*65535.0f Z2 65535)G1=65535;break;}Gz[-4]=(unsigned short)
G1;G1=Gy[1]*65535.0f Z2 65535)G1=65535;break;}Gz[-3]=(unsigned short)G1;G1=Gy[0]
*65535.0f Z2 65535)G1=65535;break;}Gz[-2]=(unsigned short)G1;G1=Gy[3]*65535.0f//
Z2 65535)G1=65535;break;}Gz[-1]=(unsigned short)G1;Gz+=4;Gy+=4;}Gz-=4;}static Z1
*DK Zj 2]);Gz[-3]=((Z1)G1[1]);Gz[-2]=((Z1)G1[0]);Gz[-1]=((Z1)G1[3]);Gz+=4;G1+=4;
}Gz-=4;return G0;}static void DL Zi 2]Z2 65535)G1=65535;break;}Gz[-4]=(unsigned
short)G1;G1=Gy[1]Z2 65535)G1=65535;break;}Gz[-3]=(unsigned short)G1;G1=Gy[0]Z2//
65535)G1=65535;break;}Gz[-2]=(unsigned short)G1;G1=Gy[3]Z2 65535)G1=65535;break;
}Gz[-1]=(unsigned short)G1;Gz+=4;Gy+=4;}Gz-=4;}static Z1*DM(Z1*Gw,int Gx,void///
const*Gy){Z1*RESTRICT Gz=Gw;Z1*G0=(Z1*)Gz+Gx;CL const*G1=(CL const*)Gy;Gz+=4;///
while(Gz<=G0){Gz[-4]=CM(G1[2]);Gz[-3]=CM(G1[1]);Gz[-2]=CM(G1[0]);Gz[-1]=CM(G1[3]
);Gz+=4;G1+=4;}Gz-=4;return G0;}static void DN(void*Gw,int Gx,Z1 const*Gy){CL*//
RESTRICT Gz=(CL*)Gw;CL*G0=((CL*)Gz)+Gx;Gz+=4;while(Gz<=G0){Gz[-4]=CN(Gy[2]);Gz[-
3]=CN(Gy[1]);Gz[-2]=CN(Gy[0]);Gz[-1]=CN(Gy[3]);Gz+=4;Gy+=4;}Gz-=4;}static Z1*DO(
Z1*Gw,int Gx,void const*Gy){Z1*RESTRICT Gz=Gw;Z1*G0=(Z1*)Gz+Gx;Z1 const*G1=(Z1//
const*)Gy;Gz+=4;while(Gz<=G0){Gz[-4]=G1[2];Gz[-3]=G1[1];Gz[-2]=G1[0];Gz[-1]=G1[3
];Gz+=4;G1+=4;}Gz-=4;return G0;}static void DP(void*Gw,int Gx,Z1 const*Gy){Z1*//
RESTRICT Gz=(Z1*)Gw;Z1*G0=((Z1*)Gz)+Gx;Gz+=4;while(Gz<=G0){Z1 G1;G1=Gy[2];Gz[-4]
=G1;G1=Gy[1];Gz[-3]=G1;G1=Gy[0];Gz[-2]=G1;G1=Gy[3];Gz[-1]=G1;Gz+=4;Gy+=4;}Gz-=4;
}static Z1*DQ Z4((Z1)G1[1])*3.922e-03;Gz[-3]=((Z1)G1[2])*3.922e-03;Gz[-2]=((Z1)
G1[3])*3.922e-03;Gz[-1]=((Z1)G1[0])*3.922e-03;Gz+=4;G1+=4;}Gz-=4;return G0;}////
static void DR Zd Gz+=4;while(Gz<=G0){Z1 G1;G1=Gy[3]*255.0f Z2 255)G1=255;break;
}Gz[-4]=(unsigned char)G1;G1=Gy[0]*255.0f Z2 255)G1=255;break;}Gz[-3]=(unsigned
char)G1;G1=Gy[1]*255.0f Z2 255)G1=255;break;}Gz[-2]=(unsigned char)G1;G1=Gy[2]*
255.0f Z2 255)G1=255;break;}Gz[-1]=(unsigned char)G1;Gz+=4;Gy+=4;}Gz-=4;}static
Z1*DS Z4((Z1)G1[1]);Gz[-3]=((Z1)G1[2]);Gz[-2]=((Z1)G1[3]);Gz[-1]=((Z1)G1[0]);Gz
+=4;G1+=4;}Gz-=4;return G0;}static void DT Zd Gz+=4;while(Gz<=G0){Z1 G1;G1=Gy[3]
Z2 255)G1=255;break;}Gz[-4]=(unsigned char)G1;G1=Gy[0]Z2 255)G1=255;break;}Gz[-3
]=(unsigned char)G1;G1=Gy[1]Z2 255)G1=255;break;}Gz[-2]=(unsigned char)G1;G1=Gy[
2]Z2 255)G1=255;break;}Gz[-1]=(unsigned char)G1;Gz+=4;Gy+=4;}Gz-=4;}static Z1*DU
Z4 CH[G1[1]];Gz[-3]=CH[G1[2]];Gz[-2]=CH[G1[3]];Gz[-1]=CH[G1[0]];Gz+=4;G1+=4;}Gz
-=4;return G0;}static void DV Zd Gz+=4;while(Gz<=G0){Gz[-4]=CK(Gy[3]);Gz[-3]=CK(
Gy[0]);Gz[-2]=CK(Gy[1]);Gz[-1]=CK(Gy[2]);Gz+=4;Gy+=4;}Gz-=4;}static Z1*DW Z3 do{
Gz[0]=CH[G1[1]];Gz[1]=CH[G1[2]];Gz[2]=CH[G1[3]];Gz[3]=((Z1)G1[0])*3.922e-03;G1+=
4;Gz+=4;}while(Gz<G0);return G0;}static void DX Zd do{Z1 G1;Gz[1]=CK(Gy[0]);Gz[2
]=CK(Gy[1]);Gz[3]=CK(Gy[2]);G1=Gy[3]*255.0f Z2 255)G1=255;break;}Gz[0]=(unsigned
char)G1;Gz+=4;Gy+=4;}while(Gz<G0);}static Z1*DY Zj 1])*1.526e-05;Gz[-3]=((Z1)G1[
2])*1.526e-05;Gz[-2]=((Z1)G1[3])*1.526e-05;Gz[-1]=((Z1)G1[0])*1.526e-05;Gz+=4;G1
+=4;}Gz-=4;return G0;}static void DZ Zi 3]*65535.0f Z2 65535)G1=65535;break;}Gz[
-4]=(unsigned short)G1;G1=Gy[0]*65535.0f Z2 65535)G1=65535;break;}Gz[-3]=(//////
unsigned short)G1;G1=Gy[1]*65535.0f Z2 65535)G1=65535;break;}Gz[-2]=(unsigned///
short)G1;G1=Gy[2]*65535.0f Z2 65535)G1=65535;break;}Gz[-1]=(unsigned short)G1;Gz
+=4;Gy+=4;}Gz-=4;}static Z1*Da Zj 1]);Gz[-3]=((Z1)G1[2]);Gz[-2]=((Z1)G1[3]);Gz[-
1]=((Z1)G1[0]);Gz+=4;G1+=4;}Gz-=4;return G0;}static void Db Zi 3]Z2 65535)G1=///
65535;break;}Gz[-4]=(unsigned short)G1;G1=Gy[0]Z2 65535)G1=65535;break;}Gz[-3]=(
unsigned short)G1;G1=Gy[1]Z2 65535)G1=65535;break;}Gz[-2]=(unsigned short)G1;G1=
Gy[2]Z2 65535)G1=65535;break;}Gz[-1]=(unsigned short)G1;Gz+=4;Gy+=4;}Gz-=4;}////
static Z1*Dc(Z1*Gw,int Gx,void const*Gy){Z1*RESTRICT Gz=Gw;Z1*G0=(Z1*)Gz+Gx;CL//
const*G1=(CL const*)Gy;Gz+=4;while(Gz<=G0){Gz[-4]=CM(G1[1]);Gz[-3]=CM(G1[2]);Gz[
-2]=CM(G1[3]);Gz[-1]=CM(G1[0]);Gz+=4;G1+=4;}Gz-=4;return G0;}static void Dd(void
*Gw,int Gx,Z1 const*Gy){CL*RESTRICT Gz=(CL*)Gw;CL*G0=((CL*)Gz)+Gx;Gz+=4;while(Gz
<=G0){Gz[-4]=CN(Gy[3]);Gz[-3]=CN(Gy[0]);Gz[-2]=CN(Gy[1]);Gz[-1]=CN(Gy[2]);Gz+=4;
Gy+=4;}Gz-=4;}static Z1*De(Z1*Gw,int Gx,void const*Gy){Z1*RESTRICT Gz=Gw;Z1*G0=(
Z1*)Gz+Gx;Z1 const*G1=(Z1 const*)Gy;Gz+=4;while(Gz<=G0){Gz[-4]=G1[1];Gz[-3]=G1[2
];Gz[-2]=G1[3];Gz[-1]=G1[0];Gz+=4;G1+=4;}Gz-=4;return G0;}static void Df(void*Gw
,int Gx,Z1 const*Gy){Z1*RESTRICT Gz=(Z1*)Gw;Z1*G0=((Z1*)Gz)+Gx;Gz+=4;while(Gz<=
G0){Z1 G1;G1=Gy[3];Gz[-4]=G1;G1=Gy[0];Gz[-3]=G1;G1=Gy[1];Gz[-2]=G1;G1=Gy[2];Gz[-
1]=G1;Gz+=4;Gy+=4;}Gz-=4;}static Z1*Dg Z4((Z1)G1[3])*3.922e-03;Gz[-3]=((Z1)G1[2]
)*3.922e-03;Gz[-2]=((Z1)G1[1])*3.922e-03;Gz[-1]=((Z1)G1[0])*3.922e-03;Gz+=4;G1+=
4;}Gz-=4;return G0;}static void Dh Zd Gz+=4;while(Gz<=G0){Z1 G1;G1=Gy[3]*255.0f
Z2 255)G1=255;break;}Gz[-4]=(unsigned char)G1;G1=Gy[2]*255.0f Z2 255)G1=255;////
break;}Gz[-3]=(unsigned char)G1;G1=Gy[1]*255.0f Z2 255)G1=255;break;}Gz[-2]=(///
unsigned char)G1;G1=Gy[0]*255.0f Z2 255)G1=255;break;}Gz[-1]=(unsigned char)G1;
Gz+=4;Gy+=4;}Gz-=4;}static Z1*Di Z4((Z1)G1[3]);Gz[-3]=((Z1)G1[2]);Gz[-2]=((Z1)G1
[1]);Gz[-1]=((Z1)G1[0]);Gz+=4;G1+=4;}Gz-=4;return G0;}static void Dj Zd Gz+=4;//
while(Gz<=G0){Z1 G1;G1=Gy[3]Z2 255)G1=255;break;}Gz[-4]=(unsigned char)G1;G1=Gy[
2]Z2 255)G1=255;break;}Gz[-3]=(unsigned char)G1;G1=Gy[1]Z2 255)G1=255;break;}Gz[
-2]=(unsigned char)G1;G1=Gy[0]Z2 255)G1=255;break;}Gz[-1]=(unsigned char)G1;Gz+=
4;Gy+=4;}Gz-=4;}static Z1*Dk Z4 CH[G1[3]];Gz[-3]=CH[G1[2]];Gz[-2]=CH[G1[1]];Gz[-
1]=CH[G1[0]];Gz+=4;G1+=4;}Gz-=4;return G0;}static void Dl Zd Gz+=4;while(Gz<=G0)
{Gz[-4]=CK(Gy[3]);Gz[-3]=CK(Gy[2]);Gz[-2]=CK(Gy[1]);Gz[-1]=CK(Gy[0]);Gz+=4;Gy+=4
;}Gz-=4;}static Z1*Dm Z3 do{Gz[0]=CH[G1[3]];Gz[1]=CH[G1[2]];Gz[2]=CH[G1[1]];Gz[3
]=((Z1)G1[0])*3.922e-03;G1+=4;Gz+=4;}while(Gz<G0);return G0;}static void Dn Zd//
do{Z1 G1;Gz[3]=CK(Gy[0]);Gz[2]=CK(Gy[1]);Gz[1]=CK(Gy[2]);G1=Gy[3]*255.0f Z2 255)
G1=255;break;}Gz[0]=(unsigned char)G1;Gz+=4;Gy+=4;}while(Gz<G0);}static Z1*Do Zj
3])*1.526e-05;Gz[-3]=((Z1)G1[2])*1.526e-05;Gz[-2]=((Z1)G1[1])*1.526e-05;Gz[-1]=(
(Z1)G1[0])*1.526e-05;Gz+=4;G1+=4;}Gz-=4;return G0;}static void Dp Zi 3]*65535.0f
Z2 65535)G1=65535;break;}Gz[-4]=(unsigned short)G1;G1=Gy[2]*65535.0f Z2 65535)G1
=65535;break;}Gz[-3]=(unsigned short)G1;G1=Gy[1]*65535.0f Z2 65535)G1=65535;////
break;}Gz[-2]=(unsigned short)G1;G1=Gy[0]*65535.0f Z2 65535)G1=65535;break;}Gz[-
1]=(unsigned short)G1;Gz+=4;Gy+=4;}Gz-=4;}static Z1*Dq Zj 3]);Gz[-3]=((Z1)G1[2])
;Gz[-2]=((Z1)G1[1]);Gz[-1]=((Z1)G1[0]);Gz+=4;G1+=4;}Gz-=4;return G0;}static void
Dr Zi 3]Z2 65535)G1=65535;break;}Gz[-4]=(unsigned short)G1;G1=Gy[2]Z2 65535)G1=
65535;break;}Gz[-3]=(unsigned short)G1;G1=Gy[1]Z2 65535)G1=65535;break;}Gz[-2]=(
unsigned short)G1;G1=Gy[0]Z2 65535)G1=65535;break;}Gz[-1]=(unsigned short)G1;Gz
+=4;Gy+=4;}Gz-=4;}static Z1*Ds(Z1*Gw,int Gx,void const*Gy){Z1*RESTRICT Gz=Gw;Z1*
G0=(Z1*)Gz+Gx;CL const*G1=(CL const*)Gy;Gz+=4;while(Gz<=G0){Gz[-4]=CM(G1[3]);Gz[
-3]=CM(G1[2]);Gz[-2]=CM(G1[1]);Gz[-1]=CM(G1[0]);Gz+=4;G1+=4;}Gz-=4;return G0;}//
static void Dt(void*Gw,int Gx,Z1 const*Gy){CL*RESTRICT Gz=(CL*)Gw;CL*G0=((CL*)Gz
)+Gx;Gz+=4;while(Gz<=G0){Gz[-4]=CN(Gy[3]);Gz[-3]=CN(Gy[2]);Gz[-2]=CN(Gy[1]);Gz[-
1]=CN(Gy[0]);Gz+=4;Gy+=4;}Gz-=4;}static Z1*Du(Z1*Gw,int Gx,void const*Gy){Z1*///
RESTRICT Gz=Gw;Z1*G0=(Z1*)Gz+Gx;Z1 const*G1=(Z1 const*)Gy;Gz+=4;while(Gz<=G0){Gz
[-4]=G1[3];Gz[-3]=G1[2];Gz[-2]=G1[1];Gz[-1]=G1[0];Gz+=4;G1+=4;}Gz-=4;return G0;}
static void Dv(void*Gw,int Gx,Z1 const*Gy){Z1*RESTRICT Gz=(Z1*)Gw;Z1*G0=((Z1*)Gz
)+Gx;Gz+=4;while(Gz<=G0){Z1 G1;G1=Gy[3];Gz[-4]=G1;G1=Gy[2];Gz[-3]=G1;G1=Gy[1];Gz
[-2]=G1;G1=Gy[0];Gz[-1]=G1;Gz+=4;Gy+=4;}Gz-=4;}static Z1*Dw Z4((Z1)G1[1])*//////
3.922e-03;Gz[-3]=((Z1)G1[0])*3.922e-03;Gz[-2]=((Z1)G1[3])*3.922e-03;Gz[-1]=((Z1)
G1[2])*3.922e-03;Gz+=4;G1+=4;}Gz-=4;while(Gz<G0){Gz[0]=((Z1)G1[1])*3.922e-03;Gz[
1]=((Z1)G1[0])*3.922e-03;Gz+=2;G1+=2;}return G0;}static void Dx Zd Gz+=4;while(
Gz<=G0){Z1 G1;G1=Gy[1]*255.0f Z2 255)G1=255;break;}Gz[-4]=(unsigned char)G1;G1=
Gy[0]*255.0f Z2 255)G1=255;break;}Gz[-3]=(unsigned char)G1;G1=Gy[3]*255.0f Z2///
255)G1=255;break;}Gz[-2]=(unsigned char)G1;G1=Gy[2]*255.0f Z2 255)G1=255;break;}
Gz[-1]=(unsigned char)G1;Gz+=4;Gy+=4;}Gz-=4;while(Gz<G0){Z1 G1;G1=Gy[1]*255.0f//
Z2 255)G1=255;break;}Gz[0]=(unsigned char)G1;G1=Gy[0]*255.0f Z2 255)G1=255;break
;}Gz[1]=(unsigned char)G1;Gz+=2;Gy+=2;}}static Z1*Dy Z4((Z1)G1[1]);Gz[-3]=((Z1)
G1[0]);Gz[-2]=((Z1)G1[3]);Gz[-1]=((Z1)G1[2]);Gz+=4;G1+=4;}Gz-=4;while(Gz<G0){Gz[
0]=((Z1)G1[1]);Gz[1]=((Z1)G1[0]);Gz+=2;G1+=2;}return G0;}static void Dz Zd Gz+=4
;while(Gz<=G0){Z1 G1;G1=Gy[1]Z2 255)G1=255;break;}Gz[-4]=(unsigned char)G1;G1=Gy
[0]Z2 255)G1=255;break;}Gz[-3]=(unsigned char)G1;G1=Gy[3]Z2 255)G1=255;break;}Gz
[-2]=(unsigned char)G1;G1=Gy[2]Z2 255)G1=255;break;}Gz[-1]=(unsigned char)G1;Gz
+=4;Gy+=4;}Gz-=4;while(Gz<G0){Z1 G1;G1=Gy[1]Z2 255)G1=255;break;}Gz[0]=(unsigned
char)G1;G1=Gy[0]Z2 255)G1=255;break;}Gz[1]=(unsigned char)G1;Gz+=2;Gy+=2;}}/////
static Z1*D0 Z4 CH[G1[1]];Gz[-3]=CH[G1[0]];Gz[-2]=CH[G1[3]];Gz[-1]=CH[G1[2]];Gz
+=4;G1+=4;}Gz-=4;while(Gz<G0){Gz[0]=CH[G1[1]];Gz[1]=CH[G1[0]];Gz+=2;G1+=2;}/////
return G0;}static void D1 Zd Gz+=4;while(Gz<=G0){Gz[-4]=CK(Gy[1]);Gz[-3]=CK(Gy[0
]);Gz[-2]=CK(Gy[3]);Gz[-1]=CK(Gy[2]);Gz+=4;Gy+=4;}Gz-=4;while(Gz<G0){Gz[0]=CK(Gy
[1]);Gz[1]=CK(Gy[0]);Gz+=2;Gy+=2;}}static Z1*D2 Z4 CH[G1[1]];Gz[-3]=((Z1)G1[0])*
3.922e-03;Gz[-2]=CH[G1[3]];Gz[-1]=((Z1)G1[2])*3.922e-03;G1+=4;Gz+=4;}Gz-=4;if(Gz
<G0){Gz[0]=CH[G1[1]];Gz[1]=((Z1)G1[0])*3.922e-03;}return G0;}static void D3 Zd//
do{Z1 G1;Gz[1]=CK(Gy[0]);G1=Gy[1]*255.0f Z2 255)G1=255;break;}Gz[0]=(unsigned///
char)G1;Gz+=2;Gy+=2;}while(Gz<G0);}static Z1*D4 Zj 1])*1.526e-05;Gz[-3]=((Z1)G1[
0])*1.526e-05;Gz[-2]=((Z1)G1[3])*1.526e-05;Gz[-1]=((Z1)G1[2])*1.526e-05;Gz+=4;G1
+=4;}Gz-=4;while(Gz<G0){Gz[0]=((Z1)G1[1])*1.526e-05;Gz[1]=((Z1)G1[0])*1.526e-05;
Gz+=2;G1+=2;}return G0;}static void D5 Zi 1]*65535.0f Z2 65535)G1=65535;break;}
Gz[-4]=(unsigned short)G1;G1=Gy[0]*65535.0f Z2 65535)G1=65535;break;}Gz[-3]=(///
unsigned short)G1;G1=Gy[3]*65535.0f Z2 65535)G1=65535;break;}Gz[-2]=(unsigned///
short)G1;G1=Gy[2]*65535.0f Z2 65535)G1=65535;break;}Gz[-1]=(unsigned short)G1;Gz
+=4;Gy+=4;}Gz-=4;while(Gz<G0){Z1 G1;G1=Gy[1]*65535.0f Z2 65535)G1=65535;break;}
Gz[0]=(unsigned short)G1;G1=Gy[0]*65535.0f Z2 65535)G1=65535;break;}Gz[1]=(/////
unsigned short)G1;Gz+=2;Gy+=2;}}static Z1*D6 Zj 1]);Gz[-3]=((Z1)G1[0]);Gz[-2]=((
Z1)G1[3]);Gz[-1]=((Z1)G1[2]);Gz+=4;G1+=4;}Gz-=4;while(Gz<G0){Gz[0]=((Z1)G1[1]);
Gz[1]=((Z1)G1[0]);Gz+=2;G1+=2;}return G0;}static void D7 Zi 1]Z2 65535)G1=65535;
break;}Gz[-4]=(unsigned short)G1;G1=Gy[0]Z2 65535)G1=65535;break;}Gz[-3]=(//////
unsigned short)G1;G1=Gy[3]Z2 65535)G1=65535;break;}Gz[-2]=(unsigned short)G1;G1=
Gy[2]Z2 65535)G1=65535;break;}Gz[-1]=(unsigned short)G1;Gz+=4;Gy+=4;}Gz-=4;while
(Gz<G0){Z1 G1;G1=Gy[1]Z2 65535)G1=65535;break;}Gz[0]=(unsigned short)G1;G1=Gy[0]
Z2 65535)G1=65535;break;}Gz[1]=(unsigned short)G1;Gz+=2;Gy+=2;}}static Z1*D8(Z1*
Gw,int Gx,void const*Gy){Z1*RESTRICT Gz=Gw;Z1*G0=(Z1*)Gz+Gx;CL const*G1=(CL/////
const*)Gy;Gz+=4;while(Gz<=G0){Gz[-4]=CM(G1[1]);Gz[-3]=CM(G1[0]);Gz[-2]=CM(G1[3])
;Gz[-1]=CM(G1[2]);Gz+=4;G1+=4;}Gz-=4;while(Gz<G0){Gz[0]=CM(G1[1]);Gz[1]=CM(G1[0]
);Gz+=2;G1+=2;}return G0;}static void D9(void*Gw,int Gx,Z1 const*Gy){CL*RESTRICT
Gz=(CL*)Gw;CL*G0=((CL*)Gz)+Gx;Gz+=4;while(Gz<=G0){Gz[-4]=CN(Gy[1]);Gz[-3]=CN(Gy[
0]);Gz[-2]=CN(Gy[3]);Gz[-1]=CN(Gy[2]);Gz+=4;Gy+=4;}Gz-=4;while(Gz<G0){Gz[0]=CN(
Gy[1]);Gz[1]=CN(Gy[0]);Gz+=2;Gy+=2;}}static Z1*D_(Z1*Gw,int Gx,void const*Gy){Z1
*RESTRICT Gz=Gw;Z1*G0=(Z1*)Gz+Gx;Z1 const*G1=(Z1 const*)Gy;Gz+=4;while(Gz<=G0){
Gz[-4]=G1[1];Gz[-3]=G1[0];Gz[-2]=G1[3];Gz[-1]=G1[2];Gz+=4;G1+=4;}Gz-=4;while(Gz<
G0){Gz[0]=G1[1];Gz[1]=G1[0];Gz+=2;G1+=2;}return G0;}static void EA(void*Gw,int//
Gx,Z1 const*Gy){Z1*RESTRICT Gz=(Z1*)Gw;Z1*G0=((Z1*)Gz)+Gx;Gz+=4;while(Gz<=G0){Z1
G1;G1=Gy[1];Gz[-4]=G1;G1=Gy[0];Gz[-3]=G1;G1=Gy[3];Gz[-2]=G1;G1=Gy[2];Gz[-1]=G1;
Gz+=4;Gy+=4;}Gz-=4;while(Gz<G0){Z1 G1;G1=Gy[1];Gz[0]=G1;G1=Gy[0];Gz[1]=G1;Gz+=2;
Gy+=2;}}static void EB(Z1*Gw,int Gx){Z1*RESTRICT Gy=Gw;Z1 const*Gz=Gw+(Gx/4)*7;
Z1*RESTRICT G0=(Z1*)Gz-Gx;while(G0<Gz){Z1 G1=G0[0],G2=G0[1],G3=G0[2],G4=G0[3];Gy
[0]=G1;Gy[1]=G2;Gy[2]=G3;Gy[3]=G4;Gy[4]=G1*G4;Gy[5]=G2*G4;Gy[6]=G3*G4;Gy+=7;G0+=
4;}}static void EC(Z1*Gw,int Gx){Z1*RESTRICT Gy=Gw;Z1 const*Gz=Gw+(Gx/2)*3;Z1*//
RESTRICT G0=(Z1*)Gz-Gx;while(G0<Gz){Z1 G1=G0[0],G2=G0[1];Gy[0]=G1;Gy[1]=G2;Gy[2]
=G1*G2;Gy+=3;G0+=2;}}static void ED(Z1*Gw,int Gx){Z1*RESTRICT Gy=Gw;Z1*RESTRICT
Gz=Gw;Z1 const*G0=Gw+Gx;do{Z1 G1=Gz[3];if(G1<7.523e-37){Gy[0]=Gz[0];Gy[1]=Gz[1];
Gy[2]=Gz[2];}else{Z1 G2=1.0f/G1;Gy[0]=Gz[4]*G2;Gy[1]=Gz[5]*G2;Gy[2]=Gz[6]*G2;}Gy
[3]=G1;Gz+=7;Gy+=4;}while(Gy<G0);}static void EE(Z1*Gw,int Gx){Z1*RESTRICT Gy=Gw
;Z1*RESTRICT Gz=Gw;Z1 const*G0=Gw+Gx;do{Z1 G1=Gz[1];Gy[0]=Gz[0];if(G1>=7.523e-37
)Gy[0]=Gz[2]/G1;Gy[1]=G1;Gz+=3;Gy+=2;}while(Gy<G0);}static void EF(Z1*Gw,int Gx)
{Z1*RESTRICT Gy=Gw;Z1 const*Gz=Gw+Gx;while(Gy<Gz){Z1 G0=Gy[3];Gy[0]*=G0;Gy[1]*=
G0;Gy[2]*=G0;Gy+=4;}}static void EG(Z1*Gw,int Gx){Z1*RESTRICT Gy=Gw;Z1 const*Gz=
Gw+Gx;while(Gy<Gz){Z1 G0=Gy[1];Gy[0]*=G0;Gy+=2;}}static void EH(Z1*Gw,int Gx){Z1
*RESTRICT Gy=Gw;Z1 const*Gz=Gw+Gx;do{Z1 G0=Gy[3];if(G0>=7.523e-37){Z1 G1=1.0f/G0
;Gy[0]*=G1;Gy[1]*=G1;Gy[2]*=G1;}Gy+=4;}while(Gy<Gz);}static void EI(Z1*Gw,int Gx
){Z1*RESTRICT Gy=Gw;Z1 const*Gz=Gw+Gx;do{Z1 G0=Gy[1];if(G0>=7.523e-37)Gy[0]/=G0;
Gy+=2;}while(Gy<Gz);}static void EJ(Z1*Gw,int Gx){Z1*RESTRICT Gy=Gw;Z1 const*Gz=
Gw+Gx;Gz-=12;while(Gy<=Gz){Z1 G0,G1,G2,G3;G0=Gy[0];G1=Gy[3];G2=Gy[6];G3=Gy[9];Gy
[0]=Gy[2];Gy[3]=Gy[5];Gy[6]=Gy[8];Gy[9]=Gy[11];Gy[2]=G0;Gy[5]=G1;Gy[8]=G2;Gy[11]
=G3;Gy+=12;}Gz+=12;while(Gy<Gz){Z1 G0=Gy[0];Gy[0]=Gy[2];Gy[2]=G0;Gy+=3;}}static
void EK(B0 const*Gw,int Gx,Z1*Gy){int Gz=Gw->d;int G0=Gw->e;int G1=B3[Gw->I]*Gz;
Bt G2=Gw->A.J;Bt G3=Gw->B.J;int G4=Cj(G3,Gx,Gw->B.E.A);const void*G5=((char*)Gw
->C)+(size_t)G4*(size_t)Gw->E;B6 const*G6=Gw->N.C;Z1*G7=Gy-Gw->N.A.A*G0;Z1*G8=0;
do{Z1*G9;void const*G_;Z1*HA;int HB;int HC;if(G6->B<G6->A)break;HC=G6->B+1-G6->A
;G9=G7+G6->A*G0;HA=G7+(G6->B+1)*G0;HB=HC*Gz;G_=((char*)G5)+G6->C*G1;if(Gw->K)G_=
Gw->K(((char*)HA)-(HC*G1)+((Gw->I!=STBIR_TYPE_Z1)?12:0),G5,HC,G6->C,G4,Gw->L);G8
=Gw->Q((Z1*)HA-HB,HB,G_);if(Gw->R)Gw->R(G9,HB);++G6;}while(G6<=(&Gw->N.C[1]));if
((G2==STBIR_EDGE_WRAP)&&(Gw->N.B[0]|Gw->N.B[1])){int G9,G_[2];int HA=Gw->A.E.A;
G_[0]=-Gw->N.B[0];G_[1]=HA;for(G9=0;G9<2;G9++){int HB=Gw->N.B[G9];if(HB){int HC=
G_[G9];Z1*HD=G7+HC*G0;Z1 const*HE=G7+Cj(G2,HC,HA)*G0;memcpy(HD,HE,HB*G0*4);if(G9
==1)G8=HD+HB*G0;}}}G8[0]=0.0f;G8[1]=0.0f;}static void EL Zf Z1 const*G5=G0;Z1 G6
;G6=G4[0]*G5[0];G3[0]=G6;G0+=G1;++Gz;G3+=1;}while(G3<G2);}static void EM Zf Z1//
const*G5=G0;Z1 G6;G6=G4[0]*G5[0];G6+=G4[1]*G5[1];G3[0]=G6;G0+=G1;++Gz;G3+=1;}///
while(G3<G2);}static void EN Zf Z1 const*G5=G0;Z1 G6;G6=G4[0]*G5[0];G6+=G4[1]*G5
[1];G6+=G4[2]*G5[2];G3[0]=G6;G0+=G1;++Gz;G3+=1;}while(G3<G2);}static void EO Zf
Z1 const*G5=G0;Z1 G6,G7,G8,G9;G6=G4[0]*G5[0];G7=G4[1]*G5[1];G8=G4[2]*G5[2];G9=G4
[3]*G5[3];G3[0]=(G6+G8)+(G7+G9);G0+=G1;++Gz;G3+=1;}while(G3<G2);}static void EP
Zf Z1 const*G5=G0;Z1 G6,G7,G8,G9;G6=G4[0]*G5[0];G7=G4[1]*G5[1];G8=G4[2]*G5[2];G9
=G4[3]*G5[3];G6+=G4[4]*G5[4];G3[0]=(G6+G8)+(G7+G9);G0+=G1;++Gz;G3+=1;}while(G3<
G2);}static void EQ Zt G3[0]=(G6+G8)+(G7+G9);G0+=G1;++Gz;G3+=1;}while(G3<G2);}//
static void ER Zt G8+=G4[6]*G5[6];G3[0]=(G6+G8)+(G7+G9);G0+=G1;++Gz;G3+=1;}while
(G3<G2);}static void ES Zt G8+=G4[6]*G5[6];G9+=G4[7]*G5[7];G3[0]=(G6+G8)+(G7+G9)
;G0+=G1;++Gz;G3+=1;}while(G3<G2);}static void ET Zt G8+=G4[6]*G5[6];G9+=G4[7]*G5
[7];G6+=G4[8]*G5[8];G3[0]=(G6+G8)+(G7+G9);G0+=G1;++Gz;G3+=1;}while(G3<G2);}/////
static void EU Zt G8+=G4[6]*G5[6];G9+=G4[7]*G5[7];G6+=G4[8]*G5[8];G7+=G4[9]*G5[9
];G3[0]=(G6+G8)+(G7+G9);G0+=G1;++Gz;G3+=1;}while(G3<G2);}static void EV Zt G8+=
G4[6]*G5[6];G9+=G4[7]*G5[7];G6+=G4[8]*G5[8];G7+=G4[9]*G5[9];G8+=G4[10]*G5[10];G3
[0]=(G6+G8)+(G7+G9);G0+=G1;++Gz;G3+=1;}while(G3<G2);}static void EW Zt G8+=G4[6]
*G5[6];G9+=G4[7]*G5[7];G6+=G4[8]*G5[8];G7+=G4[9]*G5[9];G8+=G4[10]*G5[10];G9+=G4[
11]*G5[11];G3[0]=(G6+G8)+(G7+G9);G0+=G1;++Gz;G3+=1;}while(G3<G2);}static void EX
Zf int G5=((Gz->B-Gz->A+1)-4 Zw G3[0]=(G7+G9)+(G8+G_);G0+=G1;++Gz;G3+=1;}while(
G3<G2);}static void EY Zf int G5=((Gz->B-Gz->A+1)-5 Zw G7+=G4[4]*G6[4];G3[0]=(G7
+G9)+(G8+G_);G0+=G1;++Gz;G3+=1;}while(G3<G2);}static void EZ Zf int G5=((Gz->B-
Gz->A+1)-6 Zw G7+=G4[4]*G6[4];G8+=G4[5]*G6[5];G3[0]=(G7+G9)+(G8+G_);G0+=G1;++Gz;
G3+=1;}while(G3<G2);}static void Ea Zf int G5=((Gz->B-Gz->A+1)-7 Zw G7+=G4[4]*G6
[4];G8+=G4[5]*G6[5];G9+=G4[6]*G6[6];G3[0]=(G7+G9)+(G8+G_);G0+=G1;++Gz;G3+=1;}///
while(G3<G2);}static CC*Eb[4]={EX,EY,EZ,Ea,};static CC*Ec[12]={EL,EM,EN,EO,EP,EQ
,ER,ES,ET,EU,EV,EW,};static void Ed Zs Z1 const*G5=G0;Z1 G6,G7,G8;G8=G5[0];G6=G4
[0]*G8;G7=G4[1]*G8;G3[0]=G6;G3[1]=G7;G0+=G1;++Gz;G3+=2;}while(G3<G2);}static////
void Ee Zs Z1 const*G5=G0;Z1 G6,G7,G8;G8=G5[0];G6=G4[0]*G8;G7=G4[1]*G8;G8=G5[1];
G6+=G4[2]*G8;G7+=G4[3]*G8;G3[0]=G6;G3[1]=G7;G0+=G1;++Gz;G3+=2;}while(G3<G2);}///
static void Ef Zs Z1 const*G5=G0;Z1 G6,G7,G8;G8=G5[0];G6=G4[0]*G8;G7=G4[1]*G8;G8
=G5[2];G6+=G4[4]*G8;G7+=G4[5]*G8;G8=G5[1];G6+=G4[2]*G8;G7+=G4[3]*G8;G3[0]=G6;G3[
1]=G7;G0+=G1;++Gz;G3+=2;}while(G3<G2);}static void Eg Zc G3[0]=(G6+G8)+(G7+G9);
G3[1]=(G_+HB)+(HA+HC);G0+=G1;++Gz;G3+=2;}while(G3<G2);}static void Eh Zc HD=G5[4
];G6+=G4[8]*HD;G_+=G4[9]*HD;G3[0]=(G6+G8)+(G7+G9);G3[1]=(G_+HB)+(HA+HC);G0+=G1;
++Gz;G3+=2;}while(G3<G2);}static void Ei Zc HD=G5[4];G6+=G4[8]*HD;G_+=G4[9]*HD;
HD=G5[5];G7+=G4[10]*HD;HA+=G4[11]*HD;G3[0]=(G6+G8)+(G7+G9);G3[1]=(G_+HB)+(HA+HC)
;G0+=G1;++Gz;G3+=2;}while(G3<G2);}static void Ej Zc HD=G5[4];G6+=G4[8]*HD;G_+=G4
[9]*HD;HD=G5[5];G7+=G4[10]*HD;HA+=G4[11]*HD;HD=G5[6];G8+=G4[12]*HD;HB+=G4[13]*HD
;G3[0]=(G6+G8)+(G7+G9);G3[1]=(G_+HB)+(HA+HC);G0+=G1;++Gz;G3+=2;}while(G3<G2);}//
static void Ek Zx G3[0]=(G6+G8)+(G7+G9);G3[1]=(G_+HB)+(HA+HC);G0+=G1;++Gz;G3+=2;
}while(G3<G2);}static void El Zx HD=G5[8];G6+=G4[16]*HD;G_+=G4[17]*HD;G3[0]=(G6+
G8)+(G7+G9);G3[1]=(G_+HB)+(HA+HC);G0+=G1;++Gz;G3+=2;}while(G3<G2);}static void//
Em Zx HD=G5[8];G6+=G4[16]*HD;G_+=G4[17]*HD;HD=G5[9];G7+=G4[18]*HD;HA+=G4[19]*HD;
G3[0]=(G6+G8)+(G7+G9);G3[1]=(G_+HB)+(HA+HC);G0+=G1;++Gz;G3+=2;}while(G3<G2);}///
static void En Zx HD=G5[8];G6+=G4[16]*HD;G_+=G4[17]*HD;HD=G5[9];G7+=G4[18]*HD;HA
+=G4[19]*HD;HD=G5[10];G8+=G4[20]*HD;HB+=G4[21]*HD;G3[0]=(G6+G8)+(G7+G9);G3[1]=(
G_+HB)+(HA+HC);G0+=G1;++Gz;G3+=2;}while(G3<G2);}static void Eo Zx HD=G5[8];G6+=
G4[16]*HD;G_+=G4[17]*HD;HD=G5[9];G7+=G4[18]*HD;HA+=G4[19]*HD;HD=G5[10];G8+=G4[20
]*HD;HB+=G4[21]*HD;HD=G5[11];G9+=G4[22]*HD;HC+=G4[23]*HD;G3[0]=(G6+G8)+(G7+G9);
G3[1]=(G_+HB)+(HA+HC);G0+=G1;++Gz;G3+=2;}while(G3<G2);}static void Ep Zs int G5=
((Gz->B-Gz->A+1)-4 Zk G3[0]=(G7+G9)+(G8+G_);G3[1]=(HA+HC)+(HB+HD);G0+=G1;++Gz;G3
+=2;}while(G3<G2);}static void Eq Zs int G5=((Gz->B-Gz->A+1)-5 Zk HE=G6[4];G7+=
G4[8]*HE;HA+=G4[9]*HE;G3[0]=(G7+G9)+(G8+G_);G3[1]=(HA+HC)+(HB+HD);G0+=G1;++Gz;G3
+=2;}while(G3<G2);}static void Er Zs int G5=((Gz->B-Gz->A+1)-6 Zk HE=G6[4];G7+=
G4[8]*HE;HA+=G4[9]*HE;HE=G6[5];G8+=G4[10]*HE;HB+=G4[11]*HE;G3[0]=(G7+G9)+(G8+G_)
;G3[1]=(HA+HC)+(HB+HD);G0+=G1;++Gz;G3+=2;}while(G3<G2);}static void Es Zs int G5
=((Gz->B-Gz->A+1)-7 Zk HE=G6[4];G7+=G4[8]*HE;HA+=G4[9]*HE;HE=G6[5];G8+=G4[10]*HE
;HB+=G4[11]*HE;HE=G6[6];G9+=G4[12]*HE;HC+=G4[13]*HE;G3[0]=(G7+G9)+(G8+G_);G3[1]=
(HA+HC)+(HB+HD);G0+=G1;++Gz;G3+=2;}while(G3<G2);}static CC*Et[4]={Ep,Eq,Er,Es,};
static CC*Eu[12]={Ed,Ee,Ef,Eg,Eh,Ei,Ej,Ek,El,Em,En,Eo,};static void Ev Zo Z1////
const*G5=G0;Z1 G6,G7,G8,G9;G9=G5[0];G6=G4[0]*G9;G7=G4[1]*G9;G8=G4[2]*G9;G3[0]=G6
;G3[1]=G7;G3[2]=G8;G0+=G1;++Gz;G3+=3;}while(G3<G2);}static void Ew Zo Z1 const*
G5=G0;Z1 G6,G7,G8,G9;G9=G5[0];G6=G4[0]*G9;G7=G4[1]*G9;G8=G4[2]*G9;G9=G5[1];G6+=
G4[3]*G9;G7+=G4[4]*G9;G8+=G4[5]*G9;G3[0]=G6;G3[1]=G7;G3[2]=G8;G0+=G1;++Gz;G3+=3;
}while(G3<G2);}static void Ex Zo Z1 const*G5=G0;Z1 G6,G7,G8,G9;G9=G5[0];G6=G4[0]
*G9;G7=G4[1]*G9;G8=G4[2]*G9;G9=G5[1];G6+=G4[3]*G9;G7+=G4[4]*G9;G8+=G4[5]*G9;G9=
G5[2];G6+=G4[6]*G9;G7+=G4[7]*G9;G8+=G4[8]*G9;G3[0]=G6;G3[1]=G7;G3[2]=G8;G0+=G1;
++Gz;G3+=3;}while(G3<G2);}static void Ey Zo Z1 const*G5=G0;Z1 G6,G7,G8,G9,G_,HA,
HB,HC,HD,HE,HF,HG,HH;HH=G5[0];G6=G4[0]*HH;G7=G4[1]*HH;G8=G4[2]*HH;HH=G5[1];G9=G4
[3]*HH;G_=G4[4]*HH;HA=G4[5]*HH;HH=G5[2];HB=G4[6]*HH;HC=G4[7]*HH;HD=G4[8]*HH;HH=
G5[3];HE=G4[9]*HH;HF=G4[10]*HH;HG=G4[11]*HH;Zq Ez Zb Zq E0 Zb HH=G5[5];G9+=G4[15
]*HH;G_+=G4[16]*HH;HA+=G4[17]*HH;Zq E1 Zb HH=G5[5];G9+=G4[15]*HH;G_+=G4[16]*HH;
HA+=G4[17]*HH;HH=G5[6];HB+=G4[18]*HH;HC+=G4[19]*HH;HD+=G4[20]*HH;Zq E2 Zb HH=G5[
5];G9+=G4[15]*HH;G_+=G4[16]*HH;HA+=G4[17]*HH;HH=G5[6];HB+=G4[18]*HH;HC+=G4[19]*
HH;HD+=G4[20]*HH;HH=G5[7];HE+=G4[21]*HH;HF+=G4[22]*HH;HG+=G4[23]*HH;Zq E3 Zv Zq
E4 Zv HH=G5[9];G9+=G4[27]*HH;G_+=G4[28]*HH;HA+=G4[29]*HH;Zq E5 Zv HH=G5[9];G9+=
G4[27]*HH;G_+=G4[28]*HH;HA+=G4[29]*HH;HH=G5[10];HB+=G4[30]*HH;HC+=G4[31]*HH;HD+=
G4[32]*HH;Zq E6 Zv HH=G5[9];G9+=G4[27]*HH;G_+=G4[28]*HH;HA+=G4[29]*HH;HH=G5[10];
HB+=G4[30]*HH;HC+=G4[31]*HH;HD+=G4[32]*HH;HH=G5[11];HE+=G4[33]*HH;HF+=G4[34]*HH;
HG+=G4[35]*HH;Zq E7 Zo int G5=((Gz->B-Gz->A+1)-4 Zg G3[0]=(G7+HC)+(G_+HF);G3[1]=
(G8+HD)+(HA+HG);G3[2]=(G9+HE)+(HB+HH);G0+=G1;++Gz;G3+=3;}while(G3<G2);}static///
void E8 Zo int G5=((Gz->B-Gz->A+1)-5 Zg HI=G6[4];G7+=G4[12]*HI;G8+=G4[13]*HI;G9
+=G4[14]*HI;G3[0]=(G7+HC)+(G_+HF);G3[1]=(G8+HD)+(HA+HG);G3[2]=(G9+HE)+(HB+HH);G0
+=G1;++Gz;G3+=3;}while(G3<G2);}static void E9 Zo int G5=((Gz->B-Gz->A+1)-6 Zg HI
=G6[4];G7+=G4[12]*HI;G8+=G4[13]*HI;G9+=G4[14]*HI;HI=G6[5];G_+=G4[15]*HI;HA+=G4[
16]*HI;HB+=G4[17]*HI;G3[0]=(G7+HC)+(G_+HF);G3[1]=(G8+HD)+(HA+HG);G3[2]=(G9+HE)+(
HB+HH);G0+=G1;++Gz;G3+=3;}while(G3<G2);}static void E_ Zo int G5=((Gz->B-Gz->A+1
)-7 Zg HI=G6[4];G7+=G4[12]*HI;G8+=G4[13]*HI;G9+=G4[14]*HI;HI=G6[5];G_+=G4[15]*HI
;HA+=G4[16]*HI;HB+=G4[17]*HI;HI=G6[6];HC+=G4[18]*HI;HD+=G4[19]*HI;HE+=G4[20]*HI;
G3[0]=(G7+HC)+(G_+HF);G3[1]=(G8+HD)+(HA+HG);G3[2]=(G9+HE)+(HB+HH);G0+=G1;++Gz;G3
+=3;}while(G3<G2);}static CC*FA[4]={E7,E8,E9,E_,};static CC*FB[12]={Ev,Ew,Ex,Ey,
Ez,E0,E1,E2,E3,E4,E5,E6,};static void FC Zp Z1 const*G5=G0;Z1 G6,G7,G8,G9,G_;G_=
G5[0];G6=G4[0]*G_;G7=G4[1]*G_;G8=G4[2]*G_;G9=G4[3]*G_;G3[0]=G6;G3[1]=G7;G3[2]=G8
;G3[3]=G9;G0+=G1;++Gz;G3+=4;}while(G3<G2);}static void FD Zp Z1 const*G5=G0;Z1//
G6,G7,G8,G9,G_;G_=G5[0];G6=G4[0]*G_;G7=G4[1]*G_;G8=G4[2]*G_;G9=G4[3]*G_;G_=G5[1]
;G6+=G4[4]*G_;G7+=G4[5]*G_;G8+=G4[6]*G_;G9+=G4[7]*G_;G3[0]=G6;G3[1]=G7;G3[2]=G8;
G3[3]=G9;G0+=G1;++Gz;G3+=4;}while(G3<G2);}static void FE Zp Z1 const*G5=G0;Z1 G6
,G7,G8,G9,G_;G_=G5[0];G6=G4[0]*G_;G7=G4[1]*G_;G8=G4[2]*G_;G9=G4[3]*G_;G_=G5[1];
G6+=G4[4]*G_;G7+=G4[5]*G_;G8+=G4[6]*G_;G9+=G4[7]*G_;G_=G5[2];G6+=G4[8]*G_;G7+=G4
[9]*G_;G8+=G4[10]*G_;G9+=G4[11]*G_;G3[0]=G6;G3[1]=G7;G3[2]=G8;G3[3]=G9;G0+=G1;++
Gz;G3+=4;}while(G3<G2);}static void FF Zp Z1 const*G5=G0;Z1 G6,G7,G8,G9,G_,HA,HB
,HC,HD;HD=G5[0];G6=G4[0]*HD;G7=G4[1]*HD;G8=G4[2]*HD;G9=G4[3]*HD;HD=G5[1];G_=G4[4
]*HD;HA=G4[5]*HD;HB=G4[6]*HD;HC=G4[7]*HD;HD=G5[2];G6+=G4[8]*HD;G7+=G4[9]*HD;G8+=
G4[10]*HD;G9+=G4[11]*HD;HD=G5[3];G_+=G4[12]*HD;HA+=G4[13]*HD;HB+=G4[14]*HD;HC+=
G4[15]*HD;G3[0]=G6+G_;G3[1]=G7+HA;G3[2]=G8+HB;G3[3]=G9+HC;G0+=G1;++Gz;G3+=4;}///
while(G3<G2);}static void FG Za G3[0]=G6+G_;G3[1]=G7+HA;G3[2]=G8+HB;G3[3]=G9+HC;
G0+=G1;++Gz;G3+=4;}while(G3<G2);}static void FH Za HD=G5[5];G_+=G4[20]*HD;HA+=G4
[21]*HD;HB+=G4[22]*HD;HC+=G4[23]*HD;G3[0]=G6+G_;G3[1]=G7+HA;G3[2]=G8+HB;G3[3]=G9
+HC;G0+=G1;++Gz;G3+=4;}while(G3<G2);}static void FI Za HD=G5[5];G_+=G4[20]*HD;HA
+=G4[21]*HD;HB+=G4[22]*HD;HC+=G4[23]*HD;HD=G5[6];G6+=G4[24]*HD;G7+=G4[25]*HD;G8
+=G4[26]*HD;G9+=G4[27]*HD;G3[0]=G6+G_;G3[1]=G7+HA;G3[2]=G8+HB;G3[3]=G9+HC;G0+=G1
;++Gz;G3+=4;}while(G3<G2);}static void FJ Za HD=G5[5];G_+=G4[20]*HD;HA+=G4[21]*
HD;HB+=G4[22]*HD;HC+=G4[23]*HD;HD=G5[6];G6+=G4[24]*HD;G7+=G4[25]*HD;G8+=G4[26]*
HD;G9+=G4[27]*HD;HD=G5[7];G_+=G4[28]*HD;HA+=G4[29]*HD;HB+=G4[30]*HD;HC+=G4[31]*
HD;G3[0]=G6+G_;G3[1]=G7+HA;G3[2]=G8+HB;G3[3]=G9+HC;G0+=G1;++Gz;G3+=4;}while(G3<
G2);}static void FK Zn G3[0]=G6+G_;G3[1]=G7+HA;G3[2]=G8+HB;G3[3]=G9+HC;G0+=G1;++
Gz;G3+=4;}while(G3<G2);}static void FL Zn HD=G5[9];G_+=G4[36]*HD;HA+=G4[37]*HD;
HB+=G4[38]*HD;HC+=G4[39]*HD;G3[0]=G6+G_;G3[1]=G7+HA;G3[2]=G8+HB;G3[3]=G9+HC;G0+=
G1;++Gz;G3+=4;}while(G3<G2);}static void FM Zn HD=G5[9];G_+=G4[36]*HD;HA+=G4[37]
*HD;HB+=G4[38]*HD;HC+=G4[39]*HD;HD=G5[10];G6+=G4[40]*HD;G7+=G4[41]*HD;G8+=G4[42]
*HD;G9+=G4[43]*HD;G3[0]=G6+G_;G3[1]=G7+HA;G3[2]=G8+HB;G3[3]=G9+HC;G0+=G1;++Gz;G3
+=4;}while(G3<G2);}static void FN Zn HD=G5[9];G_+=G4[36]*HD;HA+=G4[37]*HD;HB+=G4
[38]*HD;HC+=G4[39]*HD;HD=G5[10];G6+=G4[40]*HD;G7+=G4[41]*HD;G8+=G4[42]*HD;G9+=G4
[43]*HD;HD=G5[11];G_+=G4[44]*HD;HA+=G4[45]*HD;HB+=G4[46]*HD;HC+=G4[47]*HD;G3[0]=
G6+G_;G3[1]=G7+HA;G3[2]=G8+HB;G3[3]=G9+HC;G0+=G1;++Gz;G3+=4;}while(G3<G2);}/////
static void FO Zp int G5=((Gz->B-Gz->A+1)-4 Ze G3[0]=G7+HA;G3[1]=G8+HB;G3[2]=G9+
HC;G3[3]=G_+HD;G0+=G1;++Gz;G3+=4;}while(G3<G2);}static void FP Zp int G5=((Gz->B
-Gz->A+1)-5 Ze HE=G6[4];G7+=G4[16]*HE;G8+=G4[17]*HE;G9+=G4[18]*HE;G_+=G4[19]*HE;
G3[0]=G7+HA;G3[1]=G8+HB;G3[2]=G9+HC;G3[3]=G_+HD;G0+=G1;++Gz;G3+=4;}while(G3<G2);
}static void FQ Zp int G5=((Gz->B-Gz->A+1)-6 Ze HE=G6[4];G7+=G4[16]*HE;G8+=G4[17
]*HE;G9+=G4[18]*HE;G_+=G4[19]*HE;HE=G6[5];HA+=G4[20]*HE;HB+=G4[21]*HE;HC+=G4[22]
*HE;HD+=G4[23]*HE;G3[0]=G7+HA;G3[1]=G8+HB;G3[2]=G9+HC;G3[3]=G_+HD;G0+=G1;++Gz;G3
+=4;}while(G3<G2);}static void FR Zp int G5=((Gz->B-Gz->A+1)-7 Ze HE=G6[4];G7+=
G4[16]*HE;G8+=G4[17]*HE;G9+=G4[18]*HE;G_+=G4[19]*HE;HE=G6[5];HA+=G4[20]*HE;HB+=
G4[21]*HE;HC+=G4[22]*HE;HD+=G4[23]*HE;HE=G6[6];G7+=G4[24]*HE;G8+=G4[25]*HE;G9+=
G4[26]*HE;G_+=G4[27]*HE;G3[0]=G7+HA;G3[1]=G8+HB;G3[2]=G9+HC;G3[3]=G_+HD;G0+=G1;
++Gz;G3+=4;}while(G3<G2);}static CC*FS[4]={FO,FP,FQ,FR,};static CC*FT[12]={FC,FD
,FE,FF,FG,FH,FI,FJ,FK,FL,FM,FN,};static void FU Z5;HC=G5[0];G6=G4[0]*HC;G7=G4[1]
*HC;G8=G4[2]*HC;G9=G4[3]*HC;G_=G4[4]*HC;HA=G4[5]*HC;HB=G4[6]*HC;G3[0]=G6;G3[1]=
G7;G3[2]=G8;G3[3]=G9;G3[4]=G_;G3[5]=HA;G3[6]=HB;G0+=G1;++Gz;G3+=7;}while(G3<G2);
}static void FV Z5;HC=G5[0];G6=G4[0]*HC;G7=G4[1]*HC;G8=G4[2]*HC;G9=G4[3]*HC;G_=
G4[4]*HC;HA=G4[5]*HC;HB=G4[6]*HC;HC=G5[1];G6+=G4[7]*HC;G7+=G4[8]*HC;G8+=G4[9]*HC
;G9+=G4[10]*HC;G_+=G4[11]*HC;HA+=G4[12]*HC;HB+=G4[13]*HC;G3[0]=G6;G3[1]=G7;G3[2]
=G8;G3[3]=G9;G3[4]=G_;G3[5]=HA;G3[6]=HB;G0+=G1;++Gz;G3+=7;}while(G3<G2);}static
void FW Z5;HC=G5[0];G6=G4[0]*HC;G7=G4[1]*HC;G8=G4[2]*HC;G9=G4[3]*HC;G_=G4[4]*HC;
HA=G4[5]*HC;HB=G4[6]*HC;HC=G5[1];G6+=G4[7]*HC;G7+=G4[8]*HC;G8+=G4[9]*HC;G9+=G4[
10]*HC;G_+=G4[11]*HC;HA+=G4[12]*HC;HB+=G4[13]*HC;HC=G5[2];G6+=G4[14]*HC;G7+=G4[
15]*HC;G8+=G4[16]*HC;G9+=G4[17]*HC;G_+=G4[18]*HC;HA+=G4[19]*HC;HB+=G4[20]*HC;G3[
0]=G6;G3[1]=G7;G3[2]=G8;G3[3]=G9;G3[4]=G_;G3[5]=HA;G3[6]=HB;G0+=G1;++Gz;G3+=7;}
while(G3<G2);}static void FX Z7 Zq FY Z8 Zq FZ Z9 Zq Fa Z9 HJ=G5[6];G6+=G4[42]*
HJ;G7+=G4[43]*HJ;G8+=G4[44]*HJ;G9+=G4[45]*HJ;G_+=G4[46]*HJ;HA+=G4[47]*HJ;HB+=G4[
48]*HJ;Zq Fb Z9 HJ=G5[6];G6+=G4[42]*HJ;G7+=G4[43]*HJ;G8+=G4[44]*HJ;G9+=G4[45]*HJ
;G_+=G4[46]*HJ;HA+=G4[47]*HJ;HB+=G4[48]*HJ;HJ=G5[7];HC+=G4[49]*HJ;HD+=G4[50]*HJ;
HE+=G4[51]*HJ;HF+=G4[52]*HJ;HG+=G4[53]*HJ;HH+=G4[54]*HJ;HI+=G4[55]*HJ;Zq Fc Zl//
Zq Fd Zl HJ=G5[9];HC+=G4[63]*HJ;HD+=G4[64]*HJ;HE+=G4[65]*HJ;HF+=G4[66]*HJ;HG+=G4
[67]*HJ;HH+=G4[68]*HJ;HI+=G4[69]*HJ;Zq Fe Zl HJ=G5[9];HC+=G4[63]*HJ;HD+=G4[64]*
HJ;HE+=G4[65]*HJ;HF+=G4[66]*HJ;HG+=G4[67]*HJ;HH+=G4[68]*HJ;HI+=G4[69]*HJ;HJ=G5[
10];G6+=G4[70]*HJ;G7+=G4[71]*HJ;G8+=G4[72]*HJ;G9+=G4[73]*HJ;G_+=G4[74]*HJ;HA+=G4
[75]*HJ;HB+=G4[76]*HJ;Zq Ff Zl HJ=G5[9];HC+=G4[63]*HJ;HD+=G4[64]*HJ;HE+=G4[65]*
HJ;HF+=G4[66]*HJ;HG+=G4[67]*HJ;HH+=G4[68]*HJ;HI+=G4[69]*HJ;HJ=G5[10];G6+=G4[70]*
HJ;G7+=G4[71]*HJ;G8+=G4[72]*HJ;G9+=G4[73]*HJ;G_+=G4[74]*HJ;HA+=G4[75]*HJ;HB+=G4[
76]*HJ;HJ=G5[11];HC+=G4[77]*HJ;HD+=G4[78]*HJ;HE+=G4[79]*HJ;HF+=G4[80]*HJ;HG+=G4[
81]*HJ;HH+=G4[82]*HJ;HI+=G4[83]*HJ;Zq Fg(Z1*Gw,unsigned int Gx,Z1 const*Gy,B4///
const*Gz,Z1 const*G0,int G1){Z1 const*G2=Gw+Gx*7;Z1*RESTRICT G3=Gw;do{Z1 const*
G4=Gy+Gz->A*7;int G5=((Gz->B-Gz->A+1)-4+3)>>2;Z1 Z0 G3[0]=G7+HD;G3[1]=G8+HE;G3[2
]=G9+HF;G3[3]=G_+HG;G3[4]=HA+HH;G3[5]=HB+HI;G3[6]=HC+HJ;G0+=G1;++Gz;G3+=7;}while
(G3<G2);}static void Fh(Z1*Gw,unsigned int Gx,Z1 const*Gy,B4 const*Gz,Z1 const*
G0,int G1){Z1 const*G2=Gw+Gx*7;Z1*RESTRICT G3=Gw;do{Z1 const*G4=Gy+Gz->A*7;int//
G5=((Gz->B-Gz->A+1)-5+3)>>2;Z1 Z0 HK=G6[4];G7+=G4[28]*HK;G8+=G4[29]*HK;G9+=G4[30
]*HK;G_+=G4[31]*HK;HA+=G4[32]*HK;HB+=G4[33]*HK;HC+=G4[34]*HK;G3[0]=G7+HD;G3[1]=
G8+HE;G3[2]=G9+HF;G3[3]=G_+HG;G3[4]=HA+HH;G3[5]=HB+HI;G3[6]=HC+HJ;G0+=G1;++Gz;G3
+=7;}while(G3<G2);}static void Fi(Z1*Gw,unsigned int Gx,Z1 const*Gy,B4 const*Gz,
Z1 const*G0,int G1){Z1 const*G2=Gw+Gx*7;Z1*RESTRICT G3=Gw;do{Z1 const*G4=Gy+Gz->
A*7;int G5=((Gz->B-Gz->A+1)-6+3)>>2;Z1 Z0 HK=G6[4];G7+=G4[28]*HK;G8+=G4[29]*HK;
G9+=G4[30]*HK;G_+=G4[31]*HK;HA+=G4[32]*HK;HB+=G4[33]*HK;HC+=G4[34]*HK;HK=G6[5];
HD+=G4[35]*HK;HE+=G4[36]*HK;HF+=G4[37]*HK;HG+=G4[38]*HK;HH+=G4[39]*HK;HI+=G4[40]
*HK;HJ+=G4[41]*HK;G3[0]=G7+HD;G3[1]=G8+HE;G3[2]=G9+HF;G3[3]=G_+HG;G3[4]=HA+HH;G3
[5]=HB+HI;G3[6]=HC+HJ;G0+=G1;++Gz;G3+=7;}while(G3<G2);}static void Fj(Z1*Gw,////
unsigned int Gx,Z1 const*Gy,B4 const*Gz,Z1 const*G0,int G1){Z1 const*G2=Gw+Gx*7;
Z1*RESTRICT G3=Gw;do{Z1 const*G4=Gy+Gz->A*7;int G5=((Gz->B-Gz->A+1)-7+3)>>2;Z1//
Z0 HK=G6[4];G7+=G4[28]*HK;G8+=G4[29]*HK;G9+=G4[30]*HK;G_+=G4[31]*HK;HA+=G4[32]*
HK;HB+=G4[33]*HK;HC+=G4[34]*HK;HK=G6[5];HD+=G4[35]*HK;HE+=G4[36]*HK;HF+=G4[37]*
HK;HG+=G4[38]*HK;HH+=G4[39]*HK;HI+=G4[40]*HK;HJ+=G4[41]*HK;HK=G6[6];G7+=G4[42]*
HK;G8+=G4[43]*HK;G9+=G4[44]*HK;G_+=G4[45]*HK;HA+=G4[46]*HK;HB+=G4[47]*HK;HC+=G4[
48]*HK;G3[0]=G7+HD;G3[1]=G8+HE;G3[2]=G9+HF;G3[3]=G_+HG;G3[4]=HA+HH;G3[5]=HB+HI;
G3[6]=HC+HJ;G0+=G1;++Gz;G3+=7;}while(G3<G2);}static CC*Fk[4]={Fg,Fh,Fi,Fj,};////
static CC*Fl[12]={FU,FV,FW,FX,FY,FZ,Fa,Fb,Fc,Fd,Fe,Ff,};static void Fm(Z1**Gw,Z1
const*Gx,Z1 const*Gy,Z1 const*Gz){Z1*RESTRICT G0=Gw[0];Z1 G1=Gx[0];while(((char*
)Gz-(char*)Gy)>=16){Z1 G2,G3,G4,G5;G2=Gy[0],G3=Gy[1],G4=Gy[2],G5=Gy[3];G0[0]=(G2
*G1);G0[1]=(G3*G1);G0[2]=(G4*G1);G0[3]=(G5*G1);Gy+=4;G0+=4;}while(Gy<Gz){Z1 G2=
Gy[0];G0[0]=(G2*G1);++Gy;++G0;}}static void Fn(Z1*Gw,Z1 const*Gx,Z1 const**Gy,Z1
const*Gz){Z1*RESTRICT G0=Gw;Z1 const*G1=Gy[0];Z1 G2=Gx[0];if((G2>=1e00)&&(G2<=//
1e00)){memcpy(G0,G1,(char*)Gz-(char*)G1);return;}while(((char*)Gz-(char*)G1)>=16
){Z1 G3,G4,G5,G6;G3=G1[0]*G2;G4=G1[1]*G2;G5=G1[2]*G2;G6=G1[3]*G2;G0[0]=G3;G0[1]=
G4;G0[2]=G5;G0[3]=G6;G0+=4;G1+=4;}while(G1<Gz){Z1 G3;G3=G1[0]*G2;G0[0]=G3;++G0;
++G1;}}static void Fo(Z1**Gw,Z1 const*Gx,Z1 const*Gy,Z1 const*Gz){Z1*RESTRICT G0
=Gw[0];Z1 G1=Gx[0];while(((char*)Gz-(char*)Gy)>=16){Z1 G2,G3,G4,G5;G2=Gy[0],G3=
Gy[1],G4=Gy[2],G5=Gy[3];G0[0]+=(G2*G1);G0[1]+=(G3*G1);G0[2]+=(G4*G1);G0[3]+=(G5*
G1);Gy+=4;G0+=4;}while(Gy<Gz){Z1 G2=Gy[0];G0[0]+=(G2*G1);++Gy;++G0;}}static void
Fp(Z1*Gw,Z1 const*Gx,Z1 const**Gy,Z1 const*Gz){Z1*RESTRICT G0=Gw;Z1 const*G1=Gy[
0];Z1 G2=Gx[0];while(((char*)Gz-(char*)G1)>=16){Z1 G3,G4,G5,G6;G3=G0[0]+G1[0]*G2
;G4=G0[1]+G1[1]*G2;G5=G0[2]+G1[2]*G2;G6=G0[3]+G1[3]*G2;G0[0]=G3;G0[1]=G4;G0[2]=
G5;G0[3]=G6;G0+=4;G1+=4;}while(G1<Gz){Z1 G3;G3=G0[0]+G1[0]*G2;G0[0]=G3;++G0;++G1
;}}static void Fq(Z1**Gw,Z1 const*Gx,Z1 const*Gy,Z1 const*Gz){Z1*RESTRICT G0=Gw[
0];Z1 G1=Gx[0];Z1*RESTRICT G2=Gw[1];Z1 G3=Gx[1];while(((char*)Gz-(char*)Gy)>=16)
{Z1 G4,G5,G6,G7;G4=Gy[0],G5=Gy[1],G6=Gy[2],G7=Gy[3];G0[0]=(G4*G1);G0[1]=(G5*G1);
G0[2]=(G6*G1);G0[3]=(G7*G1);G2[0]=(G4*G3);G2[1]=(G5*G3);G2[2]=(G6*G3);G2[3]=(G7*
G3);Gy+=4;G0+=4;G2+=4;}while(Gy<Gz){Z1 G4=Gy[0];G0[0]=(G4*G1);G2[0]=(G4*G3);++Gy
;++G0;++G2;}}static void Fr(Z1*Gw,Z1 const*Gx,Z1 const**Gy,Z1 const*Gz){Z1*/////
RESTRICT G0=Gw;Z1 const*G1=Gy[0];Z1 G2=Gx[0];Z1 const*G3=Gy[1];Z1 G4=Gx[1];while
(((char*)Gz-(char*)G1)>=16){Z1 G5,G6,G7,G8;G5=G1[0]*G2;G6=G1[1]*G2;G7=G1[2]*G2;
G8=G1[3]*G2;G5+=G3[0]*G4;G6+=G3[1]*G4;G7+=G3[2]*G4;G8+=G3[3]*G4;G0[0]=G5;G0[1]=
G6;G0[2]=G7;G0[3]=G8;G0+=4;G1+=4;G3+=4;}while(G1<Gz){Z1 G5;G5=G1[0]*G2;G5+=G3[0]
*G4;G0[0]=G5;++G0;++G1;++G3;}}static void Fs(Z1**Gw,Z1 const*Gx,Z1 const*Gy,Z1//
const*Gz){Z1*RESTRICT G0=Gw[0];Z1 G1=Gx[0];Z1*RESTRICT G2=Gw[1];Z1 G3=Gx[1];////
while(((char*)Gz-(char*)Gy)>=16){Z1 G4,G5,G6,G7;G4=Gy[0],G5=Gy[1],G6=Gy[2],G7=Gy
[3];G0[0]+=(G4*G1);G0[1]+=(G5*G1);G0[2]+=(G6*G1);G0[3]+=(G7*G1);G2[0]+=(G4*G3);
G2[1]+=(G5*G3);G2[2]+=(G6*G3);G2[3]+=(G7*G3);Gy+=4;G0+=4;G2+=4;}while(Gy<Gz){Z1
G4=Gy[0];G0[0]+=(G4*G1);G2[0]+=(G4*G3);++Gy;++G0;++G2;}}static void Ft(Z1*Gw,Z1
const*Gx,Z1 const**Gy,Z1 const*Gz){Z1*RESTRICT G0=Gw;Z1 const*G1=Gy[0];Z1 G2=Gx[
0];Z1 const*G3=Gy[1];Z1 G4=Gx[1];while(((char*)Gz-(char*)G1)>=16){Z1 G5,G6,G7,G8
;G5=G0[0]+G1[0]*G2;G6=G0[1]+G1[1]*G2;G7=G0[2]+G1[2]*G2;G8=G0[3]+G1[3]*G2;G5+=G3[
0]*G4;G6+=G3[1]*G4;G7+=G3[2]*G4;G8+=G3[3]*G4;G0[0]=G5;G0[1]=G6;G0[2]=G7;G0[3]=G8
;G0+=4;G1+=4;G3+=4;}while(G1<Gz){Z1 G5;G5=G0[0]+G1[0]*G2;G5+=G3[0]*G4;G0[0]=G5;
++G0;++G1;++G3;}}static void Fu(Z1**Gw,Z1 const*Gx,Z1 const*Gy,Z1 const*Gz){Z1*
RESTRICT G0=Gw[0];Z1 G1=Gx[0];Z1*RESTRICT G2=Gw[1];Z1 G3=Gx[1];Z1*RESTRICT G4=Gw
[2];Z1 G5=Gx[2];while(((char*)Gz-(char*)Gy)>=16){Z1 G6,G7,G8,G9;G6=Gy[0],G7=Gy[1
],G8=Gy[2],G9=Gy[3];G0[0]=(G6*G1);G0[1]=(G7*G1);G0[2]=(G8*G1);G0[3]=(G9*G1);G2[0
]=(G6*G3);G2[1]=(G7*G3);G2[2]=(G8*G3);G2[3]=(G9*G3);G4[0]=(G6*G5);G4[1]=(G7*G5);
G4[2]=(G8*G5);G4[3]=(G9*G5);Gy+=4;G0+=4;G2+=4;G4+=4;}while(Gy<Gz){Z1 G6=Gy[0];G0
[0]=(G6*G1);G2[0]=(G6*G3);G4[0]=(G6*G5);++Gy;++G0;++G2;++G4;}}static void Fv(Z1*
Gw,Z1 const*Gx,Z1 const**Gy,Z1 const*Gz){Z1*RESTRICT G0=Gw;Z1 const*G1=Gy[0];Z1
G2=Gx[0];Z1 const*G3=Gy[1];Z1 G4=Gx[1];Z1 const*G5=Gy[2];Z1 G6=Gx[2];while(((///
char*)Gz-(char*)G1)>=16){Z1 G7,G8,G9,G_;G7=G1[0]*G2;G8=G1[1]*G2;G9=G1[2]*G2;G_=
G1[3]*G2;G7+=G3[0]*G4;G8+=G3[1]*G4;G9+=G3[2]*G4;G_+=G3[3]*G4;G7+=G5[0]*G6;G8+=G5
[1]*G6;G9+=G5[2]*G6;G_+=G5[3]*G6;G0[0]=G7;G0[1]=G8;G0[2]=G9;G0[3]=G_;G0+=4;G1+=4
;G3+=4;G5+=4;}while(G1<Gz){Z1 G7;G7=G1[0]*G2;G7+=G3[0]*G4;G7+=G5[0]*G6;G0[0]=G7;
++G0;++G1;++G3;++G5;}}static void Fw(Z1**Gw,Z1 const*Gx,Z1 const*Gy,Z1 const*Gz)
{Z1*RESTRICT G0=Gw[0];Z1 G1=Gx[0];Z1*RESTRICT G2=Gw[1];Z1 G3=Gx[1];Z1*RESTRICT//
G4=Gw[2];Z1 G5=Gx[2];while(((char*)Gz-(char*)Gy)>=16){Z1 G6,G7,G8,G9;G6=Gy[0],G7
=Gy[1],G8=Gy[2],G9=Gy[3];G0[0]+=(G6*G1);G0[1]+=(G7*G1);G0[2]+=(G8*G1);G0[3]+=(G9
*G1);G2[0]+=(G6*G3);G2[1]+=(G7*G3);G2[2]+=(G8*G3);G2[3]+=(G9*G3);G4[0]+=(G6*G5);
G4[1]+=(G7*G5);G4[2]+=(G8*G5);G4[3]+=(G9*G5);Gy+=4;G0+=4;G2+=4;G4+=4;}while(Gy<
Gz){Z1 G6=Gy[0];G0[0]+=(G6*G1);G2[0]+=(G6*G3);G4[0]+=(G6*G5);++Gy;++G0;++G2;++G4
;}}static void Fx(Z1*Gw,Z1 const*Gx,Z1 const**Gy,Z1 const*Gz){Z1*RESTRICT G0=Gw;
Z1 const*G1=Gy[0];Z1 G2=Gx[0];Z1 const*G3=Gy[1];Z1 G4=Gx[1];Z1 const*G5=Gy[2];Z1
G6=Gx[2];while(((char*)Gz-(char*)G1)>=16){Z1 G7,G8,G9,G_;G7=G0[0]+G1[0]*G2;G8=G0
[1]+G1[1]*G2;G9=G0[2]+G1[2]*G2;G_=G0[3]+G1[3]*G2;G7+=G3[0]*G4;G8+=G3[1]*G4;G9+=
G3[2]*G4;G_+=G3[3]*G4;G7+=G5[0]*G6;G8+=G5[1]*G6;G9+=G5[2]*G6;G_+=G5[3]*G6;G0[0]=
G7;G0[1]=G8;G0[2]=G9;G0[3]=G_;G0+=4;G1+=4;G3+=4;G5+=4;}while(G1<Gz){Z1 G7;G7=G0[
0]+G1[0]*G2;G7+=G3[0]*G4;G7+=G5[0]*G6;G0[0]=G7;++G0;++G1;++G3;++G5;}}static void
Fy Zh while(((char*)Gz-(char*)Gy)>=16){Z1 G8,G9,G_,HA;G8=Gy[0],G9=Gy[1],G_=Gy[2]
,HA=Gy[3];G0[0]=(G8*G1);G0[1]=(G9*G1);G0[2]=(G_*G1);G0[3]=(HA*G1);G2[0]=(G8*G3);
G2[1]=(G9*G3);G2[2]=(G_*G3);G2[3]=(HA*G3);G4[0]=(G8*G5);G4[1]=(G9*G5);G4[2]=(G_*
G5);G4[3]=(HA*G5);G6[0]=(G8*G7);G6[1]=(G9*G7);G6[2]=(G_*G7);G6[3]=(HA*G7);Gy+=4;
G0+=4;G2+=4;G4+=4;G6+=4;}while(Gy<Gz){Z1 G8=Gy[0];G0[0]=(G8*G1);G2[0]=(G8*G3);G4
[0]=(G8*G5);G6[0]=(G8*G7);++Gy;++G0;++G2;++G4;++G6;}}static void Fz Z6 while(((
char*)Gz-(char*)G1)>=16){Z1 G9,G_,HA,HB;G9=G1[0]*G2;G_=G1[1]*G2;HA=G1[2]*G2;HB=
G1[3]*G2;G9+=G3[0]*G4;G_+=G3[1]*G4;HA+=G3[2]*G4;HB+=G3[3]*G4;G9+=G5[0]*G6;G_+=G5
[1]*G6;HA+=G5[2]*G6;HB+=G5[3]*G6;G9+=G7[0]*G8;G_+=G7[1]*G8;HA+=G7[2]*G8;HB+=G7[3
]*G8;G0[0]=G9;G0[1]=G_;G0[2]=HA;G0[3]=HB;G0+=4;G1+=4;G3+=4;G5+=4;G7+=4;}while(G1
<Gz){Z1 G9;G9=G1[0]*G2;G9+=G3[0]*G4;G9+=G5[0]*G6;G9+=G7[0]*G8;G0[0]=G9;++G0;++G1
;++G3;++G5;++G7;}}static void F0 Zh while(((char*)Gz-(char*)Gy)>=16){Z1 G8,G9,G_
,HA;G8=Gy[0],G9=Gy[1],G_=Gy[2],HA=Gy[3];G0[0]+=(G8*G1);G0[1]+=(G9*G1);G0[2]+=(G_
*G1);G0[3]+=(HA*G1);G2[0]+=(G8*G3);G2[1]+=(G9*G3);G2[2]+=(G_*G3);G2[3]+=(HA*G3);
G4[0]+=(G8*G5);G4[1]+=(G9*G5);G4[2]+=(G_*G5);G4[3]+=(HA*G5);G6[0]+=(G8*G7);G6[1]
+=(G9*G7);G6[2]+=(G_*G7);G6[3]+=(HA*G7);Gy+=4;G0+=4;G2+=4;G4+=4;G6+=4;}while(Gy<
Gz){Z1 G8=Gy[0];G0[0]+=(G8*G1);G2[0]+=(G8*G3);G4[0]+=(G8*G5);G6[0]+=(G8*G7);++Gy
;++G0;++G2;++G4;++G6;}}static void F1 Z6 while(((char*)Gz-(char*)G1)>=16){Z1 G9,
G_,HA,HB;G9=G0[0]+G1[0]*G2;G_=G0[1]+G1[1]*G2;HA=G0[2]+G1[2]*G2;HB=G0[3]+G1[3]*G2
;G9+=G3[0]*G4;G_+=G3[1]*G4;HA+=G3[2]*G4;HB+=G3[3]*G4;G9+=G5[0]*G6;G_+=G5[1]*G6;
HA+=G5[2]*G6;HB+=G5[3]*G6;G9+=G7[0]*G8;G_+=G7[1]*G8;HA+=G7[2]*G8;HB+=G7[3]*G8;G0
[0]=G9;G0[1]=G_;G0[2]=HA;G0[3]=HB;G0+=4;G1+=4;G3+=4;G5+=4;G7+=4;}while(G1<Gz){Z1
G9;G9=G0[0]+G1[0]*G2;G9+=G3[0]*G4;G9+=G5[0]*G6;G9+=G7[0]*G8;G0[0]=G9;++G0;++G1;
++G3;++G5;++G7;}}static void F2 Zh Z1*RESTRICT G8=Gw[4];Z1 G9=Gx[4];while(((char
*)Gz-(char*)Gy)>=16){Z1 G_,HA,HB,HC;G_=Gy[0],HA=Gy[1],HB=Gy[2],HC=Gy[3];G0[0]=(
G_*G1);G0[1]=(HA*G1);G0[2]=(HB*G1);G0[3]=(HC*G1);G2[0]=(G_*G3);G2[1]=(HA*G3);G2[
2]=(HB*G3);G2[3]=(HC*G3);G4[0]=(G_*G5);G4[1]=(HA*G5);G4[2]=(HB*G5);G4[3]=(HC*G5)
;G6[0]=(G_*G7);G6[1]=(HA*G7);G6[2]=(HB*G7);G6[3]=(HC*G7);G8[0]=(G_*G9);G8[1]=(HA
*G9);G8[2]=(HB*G9);G8[3]=(HC*G9);Gy+=4;G0+=4;G2+=4;G4+=4;G6+=4;G8+=4;}while(Gy<
Gz){Z1 G_=Gy[0];G0[0]=(G_*G1);G2[0]=(G_*G3);G4[0]=(G_*G5);G6[0]=(G_*G7);G8[0]=(
G_*G9);++Gy;++G0;++G2;++G4;++G6;++G8;}}static void F3 Z6 Z1 const*G9=Gy[4];Z1 G_
=Gx[4];while(((char*)Gz-(char*)G1)>=16){Z1 HA,HB,HC,HD;HA=G1[0]*G2;HB=G1[1]*G2;
HC=G1[2]*G2;HD=G1[3]*G2;HA+=G3[0]*G4;HB+=G3[1]*G4;HC+=G3[2]*G4;HD+=G3[3]*G4;HA+=
G5[0]*G6;HB+=G5[1]*G6;HC+=G5[2]*G6;HD+=G5[3]*G6;HA+=G7[0]*G8;HB+=G7[1]*G8;HC+=G7
[2]*G8;HD+=G7[3]*G8;HA+=G9[0]*G_;HB+=G9[1]*G_;HC+=G9[2]*G_;HD+=G9[3]*G_;G0[0]=HA
;G0[1]=HB;G0[2]=HC;G0[3]=HD;G0+=4;G1+=4;G3+=4;G5+=4;G7+=4;G9+=4;}while(G1<Gz){Z1
HA;HA=G1[0]*G2;HA+=G3[0]*G4;HA+=G5[0]*G6;HA+=G7[0]*G8;HA+=G9[0]*G_;G0[0]=HA;++G0
;++G1;++G3;++G5;++G7;++G9;}}static void F4 Zh Z1*RESTRICT G8=Gw[4];Z1 G9=Gx[4];
while(((char*)Gz-(char*)Gy)>=16){Z1 G_,HA,HB,HC;G_=Gy[0],HA=Gy[1],HB=Gy[2],HC=Gy
[3];G0[0]+=(G_*G1);G0[1]+=(HA*G1);G0[2]+=(HB*G1);G0[3]+=(HC*G1);G2[0]+=(G_*G3);
G2[1]+=(HA*G3);G2[2]+=(HB*G3);G2[3]+=(HC*G3);G4[0]+=(G_*G5);G4[1]+=(HA*G5);G4[2]
+=(HB*G5);G4[3]+=(HC*G5);G6[0]+=(G_*G7);G6[1]+=(HA*G7);G6[2]+=(HB*G7);G6[3]+=(HC
*G7);G8[0]+=(G_*G9);G8[1]+=(HA*G9);G8[2]+=(HB*G9);G8[3]+=(HC*G9);Gy+=4;G0+=4;G2
+=4;G4+=4;G6+=4;G8+=4;}while(Gy<Gz){Z1 G_=Gy[0];G0[0]+=(G_*G1);G2[0]+=(G_*G3);G4
[0]+=(G_*G5);G6[0]+=(G_*G7);G8[0]+=(G_*G9);++Gy;++G0;++G2;++G4;++G6;++G8;}}/////
static void F5 Z6 Z1 const*G9=Gy[4];Z1 G_=Gx[4];while(((char*)Gz-(char*)G1)>=16)
{Z1 HA,HB,HC,HD;HA=G0[0]+G1[0]*G2;HB=G0[1]+G1[1]*G2;HC=G0[2]+G1[2]*G2;HD=G0[3]+
G1[3]*G2;HA+=G3[0]*G4;HB+=G3[1]*G4;HC+=G3[2]*G4;HD+=G3[3]*G4;HA+=G5[0]*G6;HB+=G5
[1]*G6;HC+=G5[2]*G6;HD+=G5[3]*G6;HA+=G7[0]*G8;HB+=G7[1]*G8;HC+=G7[2]*G8;HD+=G7[3
]*G8;HA+=G9[0]*G_;HB+=G9[1]*G_;HC+=G9[2]*G_;HD+=G9[3]*G_;G0[0]=HA;G0[1]=HB;G0[2]
=HC;G0[3]=HD;G0+=4;G1+=4;G3+=4;G5+=4;G7+=4;G9+=4;}while(G1<Gz){Z1 HA;HA=G0[0]+G1
[0]*G2;HA+=G3[0]*G4;HA+=G5[0]*G6;HA+=G7[0]*G8;HA+=G9[0]*G_;G0[0]=HA;++G0;++G1;++
G3;++G5;++G7;++G9;}}static void F6 Zh Z1*RESTRICT G8=Gw[4];Z1 G9=Gx[4];Z1*//////
RESTRICT G_=Gw[5];Z1 HA=Gx[5];while(((char*)Gz-(char*)Gy)>=16){Z1 HB,HC,HD,HE;HB
=Gy[0],HC=Gy[1],HD=Gy[2],HE=Gy[3];G0[0]=(HB*G1);G0[1]=(HC*G1);G0[2]=(HD*G1);G0[3
]=(HE*G1);G2[0]=(HB*G3);G2[1]=(HC*G3);G2[2]=(HD*G3);G2[3]=(HE*G3);G4[0]=(HB*G5);
G4[1]=(HC*G5);G4[2]=(HD*G5);G4[3]=(HE*G5);G6[0]=(HB*G7);G6[1]=(HC*G7);G6[2]=(HD*
G7);G6[3]=(HE*G7);G8[0]=(HB*G9);G8[1]=(HC*G9);G8[2]=(HD*G9);G8[3]=(HE*G9);G_[0]=
(HB*HA);G_[1]=(HC*HA);G_[2]=(HD*HA);G_[3]=(HE*HA);Gy+=4;G0+=4;G2+=4;G4+=4;G6+=4;
G8+=4;G_+=4;}while(Gy<Gz){Z1 HB=Gy[0];G0[0]=(HB*G1);G2[0]=(HB*G3);G4[0]=(HB*G5);
G6[0]=(HB*G7);G8[0]=(HB*G9);G_[0]=(HB*HA);++Gy;++G0;++G2;++G4;++G6;++G8;++G_;}}
static void F7 Z6 Z1 const*G9=Gy[4];Z1 G_=Gx[4];Z1 const*HA=Gy[5];Z1 HB=Gx[5];//
while(((char*)Gz-(char*)G1)>=16){Z1 HC,HD,HE,HF;HC=G1[0]*G2;HD=G1[1]*G2;HE=G1[2]
*G2;HF=Zy G1[0]*G2;HC+=G3[0]*G4;HC+=G5[0]*G6;HC+=G7[0]*G8;HC+=G9[0]*G_;HC+=HA[0]
*HB;G0[0]=HC;++G0;++G1;++G3;++G5;++G7;++G9;++HA;}}static void F8 Zh Z1*RESTRICT
G8=Gw[4];Z1 G9=Gx[4];Z1*RESTRICT G_=Gw[5];Z1 HA=Gx[5];while(((char*)Gz-(char*)Gy
)>=16){Z1 HB,HC,HD,HE;HB=Gy[0],HC=Gy[1],HD=Gy[2],HE=Gy[3];G0[0]+=(HB*G1);G0[1]+=
(HC*G1);G0[2]+=(HD*G1);G0[3]+=(HE*G1);G2[0]+=(HB*G3);G2[1]+=(HC*G3);G2[2]+=(HD*
G3);G2[3]+=(HE*G3);G4[0]+=(HB*G5);G4[1]+=(HC*G5);G4[2]+=(HD*G5);G4[3]+=(HE*G5);
G6[0]+=(HB*G7);G6[1]+=(HC*G7);G6[2]+=(HD*G7);G6[3]+=(HE*G7);G8[0]+=(HB*G9);G8[1]
+=(HC*G9);G8[2]+=(HD*G9);G8[3]+=(HE*G9);G_[0]+=(HB*HA);G_[1]+=(HC*HA);G_[2]+=(HD
*HA);G_[3]+=(HE*HA);Gy+=4;G0+=4;G2+=4;G4+=4;G6+=4;G8+=4;G_+=4;}while(Gy<Gz){Z1//
HB=Gy[0];G0[0]+=(HB*G1);G2[0]+=(HB*G3);G4[0]+=(HB*G5);G6[0]+=(HB*G7);G8[0]+=(HB*
G9);G_[0]+=(HB*HA);++Gy;++G0;++G2;++G4;++G6;++G8;++G_;}}static void F9 Z6 Z1////
const*G9=Gy[4];Z1 G_=Gx[4];Z1 const*HA=Gy[5];Z1 HB=Gx[5];while(((char*)Gz-(char*
)G1)>=16){Z1 HC,HD,HE,HF;HC=G0[0]+G1[0]*G2;HD=G0[1]+G1[1]*G2;HE=G0[2]+G1[2]*G2;
HF=G0[3]+Zy G0[0]+G1[0]*G2;HC+=G3[0]*G4;HC+=G5[0]*G6;HC+=G7[0]*G8;HC+=G9[0]*G_;
HC+=HA[0]*HB;G0[0]=HC;++G0;++G1;++G3;++G5;++G7;++G9;++HA;}}static void F_ Zh Z1*
RESTRICT G8=Gw[4];Z1 G9=Gx[4];Z1*RESTRICT G_=Gw[5];Z1 HA=Gx[5];Z1*RESTRICT HB=Gw
[6];Z1 HC=Gx[6];while(((char*)Gz-(char*)Gy)>=16){Z1 HD,HE,HF,HG;HD=Gy[0],HE=Gy[1
],HF=Gy[2],HG=Gy[3];G0[0]=(HD*G1);G0[1]=(HE*G1);G0[2]=(HF*G1);G0[3]=(HG*G1);G2[0
]=(HD*G3);G2[1]=(HE*G3);G2[2]=(HF*G3);G2[3]=(HG*G3);G4[0]=(HD*G5);G4[1]=(HE*G5);
G4[2]=(HF*G5);G4[3]=(HG*G5);G6[0]=(HD*G7);G6[1]=(HE*G7);G6[2]=(HF*G7);G6[3]=(HG*
G7);G8[0]=(HD*G9);G8[1]=(HE*G9);G8[2]=(HF*G9);G8[3]=(HG*G9);G_[0]=(HD*HA);G_[1]=
(HE*HA);G_[2]=(HF*HA);G_[3]=(HG*HA);HB[0]=(HD*HC);HB[1]=(HE*HC);HB[2]=(HF*HC);HB
[3]=(HG*HC);Gy+=4;G0+=4;G2+=4;G4+=4;G6+=4;G8+=4;G_+=4;HB+=4;}while(Gy<Gz){Z1 HD=
Gy[0];G0[0]=(HD*G1);G2[0]=(HD*G3);G4[0]=(HD*G5);G6[0]=(HD*G7);G8[0]=(HD*G9);G_[0
]=(HD*HA);HB[0]=(HD*HC);++Gy;++G0;++G2;++G4;++G6;++G8;++G_;++HB;}}static void GA
Z6 Z1 const*G9=Gy[4];Z1 G_=Gx[4];Z1 const*HA=Gy[5];Z1 HB=Gx[5];Z1 const*HC=Gy[6]
;Z1 HD=Gx[6];while(((char*)Gz-(char*)G1)>=16){Z1 HE,HF,HG,HH;HE=G1[0]*G2;HF=G1[1
]*G2;HG=G1[2]*G2;HH=Zu G1[0]*G2;HE+=G3[0]*G4;HE+=G5[0]*G6;HE+=G7[0]*G8;HE+=G9[0]
*G_;HE+=HA[0]*HB;HE+=HC[0]*HD;G0[0]=HE;++G0;++G1;++G3;++G5;++G7;++G9;++HA;++HC;}
}static void GB Zh Z1*RESTRICT G8=Gw[4];Z1 G9=Gx[4];Z1*RESTRICT G_=Gw[5];Z1 HA=
Gx[5];Z1*RESTRICT HB=Gw[6];Z1 HC=Gx[6];while(((char*)Gz-(char*)Gy)>=16){Z1 HD,HE
,HF,HG;HD=Gy[0],HE=Gy[1],HF=Gy[2],HG=Gy[3];G0[0]+=(HD*G1);G0[1]+=(HE*G1);G0[2]+=
(HF*G1);G0[3]+=(HG*G1);G2[0]+=(HD*G3);G2[1]+=(HE*G3);G2[2]+=(HF*G3);G2[3]+=(HG*
G3);G4[0]+=(HD*G5);G4[1]+=(HE*G5);G4[2]+=(HF*G5);G4[3]+=(HG*G5);G6[0]+=(HD*G7);
G6[1]+=(HE*G7);G6[2]+=(HF*G7);G6[3]+=(HG*G7);G8[0]+=(HD*G9);G8[1]+=(HE*G9);G8[2]
+=(HF*G9);G8[3]+=(HG*G9);G_[0]+=(HD*HA);G_[1]+=(HE*HA);G_[2]+=(HF*HA);G_[3]+=(HG
*HA);HB[0]+=(HD*HC);HB[1]+=(HE*HC);HB[2]+=(HF*HC);HB[3]+=(HG*HC);Gy+=4;G0+=4;G2
+=4;G4+=4;G6+=4;G8+=4;G_+=4;HB+=4;}while(Gy<Gz){Z1 HD=Gy[0];G0[0]+=(HD*G1);G2[0]
+=(HD*G3);G4[0]+=(HD*G5);G6[0]+=(HD*G7);G8[0]+=(HD*G9);G_[0]+=(HD*HA);HB[0]+=(HD
*HC);++Gy;++G0;++G2;++G4;++G6;++G8;++G_;++HB;}}static void GC Z6 Z1 const*G9=Gy[
4];Z1 G_=Gx[4];Z1 const*HA=Gy[5];Z1 HB=Gx[5];Z1 const*HC=Gy[6];Z1 HD=Gx[6];while
(((char*)Gz-(char*)G1)>=16){Z1 HE,HF,HG,HH;HE=G0[0]+G1[0]*G2;HF=G0[1]+G1[1]*G2;
HG=G0[2]+G1[2]*G2;HH=G0[3]+Zu G0[0]+G1[0]*G2;HE+=G3[0]*G4;HE+=G5[0]*G6;HE+=G7[0]
*G8;HE+=G9[0]*G_;HE+=HA[0]*HB;HE+=HC[0]*HD;G0[0]=HE;++G0;++G1;++G3;++G5;++G7;++
G9;++HA;++HC;}}static void GD Zh Z1*RESTRICT G8=Gw[4];Z1 G9=Gx[4];Z1*RESTRICT G_
=Gw[5];Z1 HA=Gx[5];Z1*RESTRICT HB=Gw[6];Z1 HC=Gx[6];Z1*RESTRICT HD=Gw[7];Z1 HE=
Gx[7];while(((char*)Gz-(char*)Gy)>=16){Z1 HF,HG,HH,HI;HF=Gy[0],HG=Gy[1],HH=Gy[2]
,HI=Gy[3];G0[0]=(HF*G1);G0[1]=(HG*G1);G0[2]=(HH*G1);G0[3]=(HI*G1);G2[0]=(HF*G3);
G2[1]=(HG*G3);G2[2]=(HH*G3);G2[3]=(HI*G3);G4[0]=(HF*G5);G4[1]=(HG*G5);G4[2]=(HH*
G5);G4[3]=(HI*G5);G6[0]=(HF*G7);G6[1]=(HG*G7);G6[2]=(HH*G7);G6[3]=(HI*G7);G8[0]=
(HF*G9);G8[1]=(HG*G9);G8[2]=(HH*G9);G8[3]=(HI*G9);G_[0]=(HF*HA);G_[1]=(HG*HA);G_
[2]=(HH*HA);G_[3]=(HI*HA);HB[0]=(HF*HC);HB[1]=(HG*HC);HB[2]=(HH*HC);HB[3]=(HI*HC
);HD[0]=(HF*HE);HD[1]=(HG*HE);HD[2]=(HH*HE);HD[3]=(HI*HE);Gy+=4;G0+=4;G2+=4;G4+=
4;G6+=4;G8+=4;G_+=4;HB+=4;HD+=4;}while(Gy<Gz){Z1 HF=Gy[0];G0[0]=(HF*G1);G2[0]=(
HF*G3);G4[0]=(HF*G5);G6[0]=(HF*G7);G8[0]=(HF*G9);G_[0]=(HF*HA);HB[0]=(HF*HC);HD[
0]=(HF*HE);++Gy;++G0;++G2;++G4;++G6;++G8;++G_;++HB;++HD;}}static void GE Z6 Z1//
const*G9=Gy[4];Z1 G_=Gx[4];Z1 const*HA=Gy[5];Z1 HB=Gx[5];Z1 const*HC=Gy[6];Z1 HD
=Gx[6];Z1 const*HE=Gy[7];Z1 HF=Gx[7];while(((char*)Gz-(char*)G1)>=16){Z1 HG,HH,
HI,HJ;HG=G1[0]*G2;HH=G1[1]*G2;HI=G1[2]*G2;HJ=Zr G1[0]*G2;HG+=G3[0]*G4;HG+=G5[0]*
G6;HG+=G7[0]*G8;HG+=G9[0]*G_;HG+=HA[0]*HB;HG+=HC[0]*HD;HG+=HE[0]*HF;G0[0]=HG;++
G0;++G1;++G3;++G5;++G7;++G9;++HA;++HC;++HE;}}static void GF Zh Z1*RESTRICT G8=Gw
[4];Z1 G9=Gx[4];Z1*RESTRICT G_=Gw[5];Z1 HA=Gx[5];Z1*RESTRICT HB=Gw[6];Z1 HC=Gx[6
];Z1*RESTRICT HD=Gw[7];Z1 HE=Gx[7];while(((char*)Gz-(char*)Gy)>=16){Z1 HF,HG,HH,
HI;HF=Gy[0],HG=Gy[1],HH=Gy[2],HI=Gy[3];G0[0]+=(HF*G1);G0[1]+=(HG*G1);G0[2]+=(HH*
G1);G0[3]+=(HI*G1);G2[0]+=(HF*G3);G2[1]+=(HG*G3);G2[2]+=(HH*G3);G2[3]+=(HI*G3);
G4[0]+=(HF*G5);G4[1]+=(HG*G5);G4[2]+=(HH*G5);G4[3]+=(HI*G5);G6[0]+=(HF*G7);G6[1]
+=(HG*G7);G6[2]+=(HH*G7);G6[3]+=(HI*G7);G8[0]+=(HF*G9);G8[1]+=(HG*G9);G8[2]+=(HH
*G9);G8[3]+=(HI*G9);G_[0]+=(HF*HA);G_[1]+=(HG*HA);G_[2]+=(HH*HA);G_[3]+=(HI*HA);
HB[0]+=(HF*HC);HB[1]+=(HG*HC);HB[2]+=(HH*HC);HB[3]+=(HI*HC);HD[0]+=(HF*HE);HD[1]
+=(HG*HE);HD[2]+=(HH*HE);HD[3]+=(HI*HE);Gy+=4;G0+=4;G2+=4;G4+=4;G6+=4;G8+=4;G_+=
4;HB+=4;HD+=4;}while(Gy<Gz){Z1 HF=Gy[0];G0[0]+=(HF*G1);G2[0]+=(HF*G3);G4[0]+=(HF
*G5);G6[0]+=(HF*G7);G8[0]+=(HF*G9);G_[0]+=(HF*HA);HB[0]+=(HF*HC);HD[0]+=(HF*HE);
++Gy;++G0;++G2;++G4;++G6;++G8;++G_;++HB;++HD;}}static void GG Z6 Z1 const*G9=Gy[
4];Z1 G_=Gx[4];Z1 const*HA=Gy[5];Z1 HB=Gx[5];Z1 const*HC=Gy[6];Z1 HD=Gx[6];Z1///
const*HE=Gy[7];Z1 HF=Gx[7];while(((char*)Gz-(char*)G1)>=16){Z1 HG,HH,HI,HJ;HG=G0
[0]+G1[0]*G2;HH=G0[1]+G1[1]*G2;HI=G0[2]+G1[2]*G2;HJ=G0[3]+Zr G0[0]+G1[0]*G2;HG+=
G3[0]*G4;HG+=G5[0]*G6;HG+=G7[0]*G8;HG+=G9[0]*G_;HG+=HA[0]*HB;HG+=HC[0]*HD;HG+=HE
[0]*HF;G0[0]=HG;++G0;++G1;++G3;++G5;++G7;++G9;++HA;++HC;++HE;}}typedef void GH(
Z1*output,Z1 const*coeffs,Z1 const**inputs,Z1 const*input0_end);static GH*GI[8]=
{Fn,Fr,Fv,Fz,F3,F7,GA,GE};static GH*GJ[8]={Fp,Ft,Fx,F1,F5,F9,GC,GG};typedef void
GK(Z1**outputs,Z1 const*coeffs,Z1 const*input,Z1 const*input_end);static GK*GL[8
]={Fm,Fq,Fu,Fy,F2,F6,F_,GD};static GK*GM[8]={Fo,Fs,Fw,F0,F4,F8,GB,GF};static////
void GN(B0 const*Gw,void*Gx,Z1*Gy,int Gz){int G0=Gw->A.E.B;int G1=Gw->d;int G2=
G0*G1;void*G3;if(Gw->T)Gw->T(Gy,G2);G3=Gx;if(Gw->M)G3=Gy;Gw->U(G3,G2,Gy);if(Gw->
M)Gw->M(G3,G0,Gz,Gw->L);}static Z1*GO(B0 const*Gw,B_ const*Gx,int Gy){return(Z1*
)(((char*)Gx->I)+(Gy*Gw->G));}static Z1*GP(B0 const*Gw,B_ const*Gx,int Gy){int//
Gz=(Gx->D+(Gy-Gx->B))%Gw->H;return GO(Gw,Gx,Gz);}static void GQ(B0 const*Gw,Z1*
Gx,Z1 const*Gy){Z1 const*Gz=Gy-(Gw->N.A.A*Gw->e);if((Gw->A.G==//////////////////
STBIR_FILTER_POINT_SAMPLE)&&(Gw->A.E.C==1.0f))memcpy(Gx,Gy,Gw->A.E.B*4*Gw->e);//
else Gw->S(Gx,Gw->A.E.B,Gz,Gw->A.A,Gw->A.B,Gw->A.K);}static void GR(B0 const*Gw,
B_*Gx,int Gy,int Gz,int G0,Z1 const*G1){Z1*G2=Gx->J;Z1*G3=Gx->A;int G4=Gw->c;int
G5=G4?(Gw->N.A.B-Gw->N.A.A+1):Gw->A.E.B;int G6=Gw->e*G5;{int G7=0,G8=G0-Gz+1;do{
Z1 const*G9[8];int G_,HA=G8;if(HA>8)HA=8;for(G_=0;G_<HA;G_++)G9[G_]=GP(Gw,Gx,G7+
G_+Gz);((G7==0)?GI:GJ)[HA-1](G4?G3:G2,G1+G7,G9,G9[0]+G6);G7+=HA;G8-=HA;}while(G8
);}if(G4){G3[G6]=0.0f;G3[G6+1]=0.0f;GQ(Gw,G2,G3);}GN(Gw,((char*)Gw->D)+((size_t)
Gy*(size_t)Gw->F),G2,Gy);}static void GS(B0 const*Gw,B_*Gx,int Gy){int Gz;Z1*G0;
EK(Gw,Gy,Gx->A);Gx->C=Gy;Gz=(Gx->D+(Gx->C-Gx->B))%Gw->H;G0=GO(Gw,Gx,Gz);GQ(Gw,G0
,Gx->A);}static void GT(B0 const*Gw,B_*Gx,int Gy){int Gz,G0,G1;B4*G2=Gw->B.A;Z1
const*G3=Gw->B.B;G0=Gx->E;G1=Gx[Gy-1].F;G2+=G0;G3+=G0*Gw->B.K;Gx->D=0;Gx->B=G2->
A;Gx->C=Gx->B-1;for(Gz=G0;Gz<G1;Gz++){int G4,G5;G4=G2->A;G5=G2->B;while(G5>Gx->C
){if((Gx->C-Gx->B+1)==Gw->H){Gx->B++;Gx->D++;}if(Gw->c){Z1*G6=GP(Gw,Gx,++Gx->C);
EK(Gw,Gx->C,G6);}else GS(Gw,Gx,Gx->C+1);}GR(Gw,Gx,Gz,G4,G5,G3);++G2;G3+=Gw->B.K;
}}static void GU(B0 const*Gw,B_*Gx){Z1*Gy=GO(Gw,Gx,Gx->D);GN(Gw,((char*)Gw->D)+(
(size_t)Gx->B*(size_t)Gw->F),Gy,Gx->B);Gy[0]=3e38;Gx->B++;if(++Gx->D==Gw->H)Gx->
D=0;}static void GV(B0 const*Gw,B_*Gx){Z1*Gy=GO(Gw,Gx,Gx->D);GQ(Gw,Gx->J,Gy);GN(
Gw,((char*)Gw->D)+((size_t)Gx->B*(size_t)Gw->F),Gx->J,Gx->B);Gy[0]=3e38;Gx->B++;
if(++Gx->D==Gw->H)Gx->D=0;}static void GW(B0 const*Gw,B_*Gx,int Gy,int Gz,Z1////
const*G0,Z1 const*G1,Z1 const*G2){{int G3=0,G4=Gz-Gy+1;do{Z1*G5[8];int G6,G7=G4;
if(G7>8)G7=8;for(G6=0;G6<G7;G6++){G5[G6]=GP(Gw,Gx,G3+G6+Gy);if(G6&&((G5[G6][0]==
3e38f)!=(G5[0][0]==3e38f))){G7=G6;break;}}(G5[0][0]==3e38f?GL:GM)[G7-1](G5,G0+G3
,G1,G2);G3+=G7;G4-=G7;}while(G4);}}typedef void GX(B0 const*stbir_info,B_*//////
split_info);static void GY(B0 const*Gw,B_*Gx,int Gy){int Gz,G0,G1,G2,G3;B4*G4=Gw
->B.A;Z1 const*G5=Gw->B.B;GX*G6;void*G7;void*G8;int G9,G_;int HA=Gw->c?(Gw->N.A.
B-Gw->N.A.A+1):Gw->A.E.B;int HB=Gw->e*HA;G0=Gx->E;G1=Gx[Gy-1].F;G2=Gx->G;G3=Gx[
Gy-1].H;Gz=G2+Gw->B.M;G4+=Gz;G5+=Gw->B.K*Gz;if(Gw->c){G6=GV;G7=Gx->A;G8=((char*)
G7)+4*Gw->e*(Gw->N.A.B-Gw->N.A.A+1);}else{G6=GU;G7=Gx->J;G8=((char*)G7)+4*Gw->e*
Gw->A.E.B;}Gx->B=G0;Gx->C=-1;Gx->D=-1;for(Gz=0;Gz<Gw->H;Gz++){Z1*HC=GO(Gw,Gx,Gz)
;HC[HB]=0.0f;HC[HB+1]=0.0f;HC[0]=3e38;}G9=1;G_=G2;for(Gz=G2;Gz<G3;Gz++){int HC,
HD;HC=G4->A;HD=G4->B;if((HD>=HC)&&(((HC>=G0)&&(HC<G1))||((HD>=G0)&&(HD<G1)))){Z1
const*HE=G5;G_=Gz;if(G9&&(Gz>G2))Gx->G=Gz;G9=0;if(HC<G0){HE+=G0-HC;HC=G0;}if(HD
>=G1)HD=G1-1;if(Gx->D<0)Gx->D=HC-G0;EK(Gw,Gz,Gx->A);if(!Gw->c)GQ(Gw,Gx->J,Gx->A)
;if(((Gx->C-Gx->B+1)==Gw->H)&&(HD>Gx->C))G6(Gw,Gx);GW(Gw,Gx,HC,HD,HE,(Z1*)G7,(Z1
*)G8);if(HD>Gx->C)Gx->C=HD;}++G4;G5+=Gw->B.K;}while(Gx->B<G1)G6(Gw,Gx);++G_;for(
Gz=0;Gz<Gy;Gz++)if(Gx[Gz].H>G_)Gx[Gz].H=G_;}static By*GZ[]={0,CP,CR,CT,CU,CV,CS}
;static Bz*Ga[]={0,CQ,CX,CZ,CZ,CZ,CW};static void Gb(B8*Gw,Bu Gx,By*Gy,Bz*Gz,Bt
G0,B7*G1,int G2,void*G3){if(Gx==0){Gx=STBIR_FILTER_MITCHELL;if(G1->C>=1.0)if((G1
->C<=1.0)&&(((Z1)ceil((Z1)G1->E))==G1->E))Gx=STBIR_FILTER_POINT_SAMPLE;else Gx=
STBIR_FILTER_CATMULLROM;}Gw->G=Gx;Gw->H=GZ[Gx];Gw->I=Ga[Gx];if(Gy&&Gz){Gw->H=Gy;
Gw->I=Gz;Gw->G=STBIR_FILTER_OTHER;}Gw->J=G0;Gw->L=Ca(Gw->I,G1->C,G3);Gw->R=0;if(
G1->C>=1.0)Gw->R=1;else if(G2||(Gw->L<=32))Gw->R=2;Gw->K=Cb(Gw,Gw->R,G3);if(G0==
STBIR_EDGE_WRAP)if(Gw->L>(G1->A*3))Gw->L=G1->A*3;Gw->M=Gw->L/2;if(G0==//////////
STBIR_EDGE_WRAP)if(Gw->M>G1->A)Gw->M=G1->A;Gw->N=Cc(Gw,Gw->R);Gw->O=Gw->N*8;Gw->
P=Gw->N*Gw->K*4+12;Gw->C=0;Gw->D=0;if(Gw->R==0){Gw->T=Gw->L;Gw->S=Cc(Gw,2);Gw->U
=Gw->S*8;Gw->V=Gw->S*Gw->T*4;}}static void Gc(B8*Gw,B4*Gx,void*Gy){Z1 Gz=Gw->E.C
;Z1 G0=Gw->E.E;Bz*G1=Gw->I;int G2=Gw->E.A;Bt G3=Gw->J;Z1 G4=Gw->E.D;if(Gw->R==1)
{int G5,G6;Z1 G7=G1(G4,Gy)*Gz;Cl(&G5,&G6,0.5,G7,G4,G0,G2,G3);Gx->A=G5;Cl(&G5,&G6
,((Z1)(Gw->E.B-1))+0.5f,G7,G4,G0,G2,G3);Gx->B=G6;}else if(Gw->R==2){Z1 G5=G1(Gz,
Gy)*G4;int G6=Gw->M;int G7=Gw->E.B;int G8;int G9;int G_,HA;Cl(&G_,&HA,0,0,G4,G0,
G2,G3);Gx->A=G_;Cl(&G_,&HA,(Z1)G7,0,G4,G0,G2,G3);Gx->B=HA;G9=Gx->A+1;G8=-G6;////
while(G9>=G8){int HB,HC;Co(&HB,&HC,((Z1)G9)+0.5f,G5,Gz,G0,G7);if(HB>HC)break;if(
(HB<G7)||(HC>=0))Gx->A=G9;--G9;}G9=Gx->B-1;G8=G9+1+G6;while(G9<=G8){int HB,HC;Co
(&HB,&HC,((Z1)G9)+0.5f,G5,Gz,G0,G7);if(HB>HC)break;if((HB<G7)||(HC>=0))Gx->B=G9;
++G9;}}if(Gw->J==STBIR_EDGE_WRAP){if((Gx->A>0)&&(Gx->B>=G2)){int G5=Gx->B-G2+1;
if((G5+16)>=Gx->A)Gx->A=0;}if((Gx->A<0)&&(Gx->B<(G2-1))){int G5=-Gx->A;if((G2-G5
-16-1)<=Gx->B)Gx->B=G2-1;}}else{if(Gx->A<0)Gx->A=0;if(Gx->B>=G2)Gx->B=G2-1;}}///
static void Gd(B_*Gw,int Gx,int Gy,int Gz,int G0,int G1,B4*G2){int G3,G4;int G5=
Gy;G4=0;for(G3=0;G3<Gx;G3++){int G6;Gw[G3].E=G4;G6=G5/(Gx-G3);Gw[G3].F=G4+G6;if(
G1&&G3){B4*G7;int G8,G9,G_,HA;B4*HB=G2+G4;G_=Gz*3;if(G6<G_)G_=G6;G9=0;G7=HB;HA=
G7->A;for(G8=1;G8<=G_;G8++){++HB;if(HB->A>HA)break;if(HB->A<G7->A){G7=HB;G9=G8;}
}Gw[G3-1].F+=G9;Gw[G3].E+=G9;}G4+=G6;G5-=G6;Gw[G3].G=-Gz;Gw[G3].H=G0+Gz;}}static
void Ge(B0*Gw){if(Gw)if(Gw->O){void*Gx=Gw->O;Gw->O=0;((void)Gw->L,free(Gx));}}//
static int Gf(int Gw,int Gx){int Gy;int Gz=0;for(Gy=0;Gy<Gw;Gy++){int G0=Gx/(Gw-
Gy);if(G0>Gz)Gz=G0;Gx-=G0;}return Gz;}static CC**Gg[8]={0,Eb,Et,FA,FS,0,0,Fk};//
static CC**Gh[8]={0,Ec,Eu,FB,FT,0,0,Fl};static Z1 Gi[5][8][4]={{{1.0,1.0,0.3125,
1.0},{0.5625,0.59375f,0.0,0.96875f},{1.0,0.0625,0.0,1.0},{0.0,0.09375f,1.0,1.0},
{1.0,1.0,0.3125,1.0},{0.03125f,0.125,1.0,1.0},{1.0,1.0,0.0625,1.0},{0.0,1.0,0.0,
0.03125f},},{{0.0,0.84375f,0.0,0.03125f},{0.09375f,0.9375,0.0,0.78125f},{0.875,
0.21875f,0.0,0.96875f},{0.09375f,0.09375f,1.0,1.0},{0.0,0.84375f,0.0,0.03125f},{
0.03125f,0.125,1.0,1.0},{1.0,1.0,0.0625,1.0},{0.0,1.0,0.0,0.53125f},},{{0.0,////
0.53125f,0.0,0.03125f},{0.0625,0.96875f,0.0,0.53125f},{0.875,0.1875,0.0,0.9375},
{0.0,0.09375f,1.0,1.0},{0.0,0.53125f,0.0,0.03125f},{0.03125f,0.125,1.0,1.0},{1.0
,1.0,0.0625,1.0},{0.0,1.0,0.0,0.5625},},{{0.0,0.5,0.0,0.71875f},{0.0625,0.84375f
,0.0,0.875},{1.0,0.5,0.5,0.96875f},{1.0,0.09375f,0.3125,0.5},{0.0,0.5,0.0,//////
0.71875f},{1.0,0.03125f,0.03125f,0.53125f},{1.0,1.0,0.0625,1.0},{0.0,1.0,///////
0.03125f,0.1875},},{{0.0,0.59375f,0.0,0.96875f},{0.0625,0.8125,0.0625,0.59375f},
{0.75,0.4375,0.125,0.96875f},{0.875,0.0625,0.1875,0.4375},{0.0,0.59375f,0.0,////
0.96875f},{0.15625f,0.125,1.0,1.0},{1.0,1.0,0.0625,1.0},{0.0,1.0,0.03125f,//////
0.34375f},}};typedef struct E{double A,B;int C;int D;int E;int F;}Gj;static int
Gk(Z1 Gw[8][4],int Gx,Z1 Gy,int Gz,int G0,Z1 G1,int G2,int G3,Gj*G4){double G5,
G6;Z1*G7;int G8;int G9;if((G2<=4)||(Gz<=4))G9=(G2<Gz)?6:7;else if((!G3)&&((G2<=
16)||(Gz<=16)))G9=4;else if(G1<=1.0f)G9=G3?1:0;else if(G1<=2.0f)G9=2;else if(G1
<=3.0f)G9=3;else G9=5;G7=Gw[G9];G6=(Z1)Gx*G7[0]+Gy*(Z1)G0*G7[1];G5=(Z1)G0*G7[2]+
G1*(Z1)Gx*G7[3];G8=(G5<=G6)?1:0;if(G4){G4->B=G6;G4->A=G5;G4->E=G9;G4->D=G8;G4->F
=G3;}if(G4&&G4->C)G8=(G4->C==2)?1:0;return G8;}static unsigned char Gl[]={1,2,3,
3,4,4,4,4,4,2,2,4,4,4,4,2,2,};static B2 Gm[]={STBIRI_BGR,STBIRI_1CHANNEL,///////
STBIRI_2CHANNEL,STBIRI_RGB,STBIRI_RGBA,STBIRI_4CHANNEL,STBIRI_BGRA,STBIRI_ARGB,
STBIRI_ABGR,STBIRI_RA,STBIRI_AR,STBIRI_RGBA_PM,STBIRI_BGRA_PM,STBIRI_ARGB_PM,///
STBIRI_ABGR_PM,STBIRI_RA_PM,STBIRI_AR_PM,};static B0*Gn(B8*Gw,B8*Gx,B4*Gy,Bs Gz,
Bs G0,int G1,int G2,int G3,int G4,void*G5){static char G6[8]={9,0,1,2,3,9,9,4};
B0*G7=0;void*G8=0;size_t G9=0;int G_;size_t HA,HB,HC,HD;int HE;int HF=0;int HG=
Gf(G1,Gx->E.B);B2 HH=Gm[Gz];B2 HI=Gm[G0];int HJ=Gl[HH];int HK=HJ;if((Gw->G!=////
STBIR_FILTER_POINT_SAMPLE)||(Gx->G!=STBIR_FILTER_POINT_SAMPLE))if((HH>=/////////
STBIRI_RGBA)&&(HH<=STBIRI_AR)&&(HI>=STBIRI_RGBA)&&(HI<=STBIRI_AR)){if(G4)HF=4;//
else{static int HL[6]={7,7,7,7,3,3};HF=2;HK=HL[HH-STBIRI_RGBA];}}else if((HH>=//
STBIRI_RGBA_PM)&&(HH<=STBIRI_AR_PM)&&(HI>=STBIRI_RGBA)&&(HI<=STBIRI_AR))HF=3;///
else if((HH>=STBIRI_RGBA)&&(HH<=STBIRI_AR)&&(HI>=STBIRI_RGBA_PM)&&(HI<=/////////
STBIRI_AR_PM))HF=1;if(HJ!=Gl[HI])return 0;G_=Gk(Gi[(int)G6[HK]],Gw->L,Gw->E.C,Gw
->E.B,Gx->L,Gx->E.C,Gx->E.B,Gx->R,0);HA=(Gy->B-Gy->A+1)*HK*4+12;HB=(size_t)Gw->E
.B*(size_t)HK*4+12;if(G_)HB=(HA+15)&~15;if((HB&4095)==0)HB+=192;HE=Gx->L+1;if((!
Gx->R)&&(HE>HG))HE=HG;HC=(size_t)HE*(size_t)HB;HD=(size_t)Gw->E.B*(size_t)HK*4+4
;for(;;){int HL;void*HM=G8;int HN=0;B8*HO=0;HM=(void*)((((size_t)HM)+15)&~15);if
(G8)G7=(B0*)HM;HM=(char*)(((size_t)HM)+512);HM=(void*)((((size_t)HM)+15)&~15);if
(G8)G7->P=(B_*)HM;HM=(char*)(((size_t)HM)+(120*G1));if(G7){static CB*HP[6]={EB,
EB,EB,EB,EC,EC};static CD*HQ[6]={ED,ED,ED,ED,EE,EE};static CB*HR[6]={EF,EF,EF,EF
,EG,EG};static CD*HS[6]={EH,EH,EH,EH,EI,EI};G7->O=G8;G7->f=G9;G7->d=HJ;G7->e=HK;
G7->a=G2;G7->b=G3;G7->V=(int)HE;G7->H=0;G7->G=(int)HB;G7->W=G1;G7->c=G_;G7->X=HH
;G7->Y=HI;G7->R=0;G7->T=0;if(HF==2){G7->R=HP[HH-STBIRI_RGBA];G7->T=HQ[HI-///////
STBIRI_RGBA];}else if(HF==4){G7->R=HR[HH-STBIRI_RGBA];G7->T=HS[HI-STBIRI_RGBA];}
else if(HF==1)G7->R=HR[HH-STBIRI_RGBA];else if(HF==3)G7->T=HS[HI-STBIRI_RGBA];if
(((HH==STBIRI_RGB)&&(HI==STBIRI_BGR))||((HH==STBIRI_BGR)&&(HI==STBIRI_RGB)))if(
Gw->E.C<1.0f)G7->T=EJ;else G7->R=EJ;}for(HL=0;HL<G1;HL++){HM=(void*)((((size_t)
HM)+15)&~15);if(G8)G7->P[HL].A=(Z1*)HM;HM=(char*)(((size_t)HM)+HA);HM=(void*)(((
(size_t)HM)+15)&~15);if(G8)G7->P[HL].I=(Z1*)HM;HM=(char*)(((size_t)HM)+HC);HM=(
void*)((((size_t)HM)+15)&~15);if(G8)G7->P[HL].J=(Z1*)HM;HM=(char*)(((size_t)HM)+
HD);}if(Gx->R==0){size_t HP;size_t HQ;HP=(size_t)Gx->U+(size_t)Gx->V;HQ=(size_t)
(HA+HC+HD)*(size_t)G1;if(HQ>=HP){if(G7){Gx->C=(B4*)G7->P[0].A;Gx->D=(Z1*)(((char
*)G7->P[0].A)+Gx->U);}}else{HM=(void*)((((size_t)HM)+15)&~15);if(G8)Gx->C=(B4*)
HM;HM=(char*)(((size_t)HM)+Gx->U);HM=(void*)((((size_t)HM)+15)&~15);if(G8)Gx->D=
(Z1*)HM;HM=(char*)(((size_t)HM)+Gx->V);}}HM=(void*)((((size_t)HM)+15)&~15);if(G8
)Gw->A=(B4*)HM;HM=(char*)(((size_t)HM)+Gw->O);HM=(void*)((((size_t)HM)+15)&~15);
if(G8)Gw->B=(Z1*)HM;HM=(char*)(((size_t)HM)+Gw->P);if((Gw->H==Gx->H)&&(Gw->I==Gx
->I)&&(Gw->J==Gx->J)&&(Gw->E.B==Gx->E.B)){Z1 HP=Gw->E.C-Gx->E.C;Z1 HQ=Gw->E.E-Gx
->E.E;if(HP<0.0f)HP=-HP;if(HQ<0.0f)HQ=-HQ;if((HP<=7.523e-37)&&(HQ<=7.523e-37)){
if(Gw->R==Gx->R){HN=1;goto no_vert_alloc;}HO=Gw;}}HM=(void*)((((size_t)HM)+15)&~
15);if(G8)Gx->A=(B4*)HM;HM=(char*)(((size_t)HM)+Gx->O);HM=(void*)((((size_t)HM)+
15)&~15);if(G8)Gx->B=(Z1*)HM;HM=(char*)(((size_t)HM)+Gx->P);no_vert_alloc:if(G7)
{Cs(Gw,0,G5);G7->S=Gg[HK][Gw->Q.C&3];if(Gw->Q.C<=12)G7->S=Gh[HK][Gw->Q.C-1];G7->
N.A.A=Gy->A;G7->N.A.B=Gy->B;Ck(Gw,&G7->N);Gw->K=Cr(Gw->N,Gw->A,Gw->B,Gw->K,Gw->Q
.C,G7->N.A.A,G7->N.A.B);memcpy(&G7->A,Gw,152);if(HN)memcpy(&G7->B,Gw,152);else{
Cs(Gx,HO,G5);memcpy(&G7->B,Gx,152);}Gd(G7->P,G7->W,G7->B.E.B,G7->B.M,G7->B.E.A,
G7->B.R,G7->B.A);G7->H=G7->B.Q.C;if((!G7->B.R)&&(G7->H>HG))G7->H=HG;}if(G7==0){
G9=(15+(size_t)HM);G8=((void)G5,malloc(G9));if(G8==0)return 0;}else return G7;}}
static int Go(B0 const*Gw,int Gx,int Gy){B_*Gz=Gw->P+Gx;if(Gw->B.R)GT(Gw,Gz,Gy);
else GY(Gw,Gz,Gy);return 1;}static void Gp(B0*Gw,B1*Gx){static CA*Gy[5]={Cx,Cx,0
,C9,C7,};static CA*Gz[6][5]={{Cz,Cx,0,C9,C7},{DG,DE,0,DO,DM},{DW,DU,0,De,Dc},{Dm
,Dk,0,Du,Ds},{C1,Cx,0,C9,C7},{D2,D0,0,D_,D8},};static CA*G0[2][2]={{Ct,Cv},{C3,
C5},};static CA*G1[6][2][2]={{{Ct,Cv},{C3,C5}},{{DA,DC},{DI,DK}},{{DQ,DS},{DY,Da
}},{{Dg,Di},{Do,Dq}},{{Ct,Cv},{C3,C5}},{{Dw,Dy},{D4,D6}}};static CE*G2[5]={Cy,Cy
,0,C_,C8,};static CE*G3[6][5]={{C0,Cy,0,C_,C8},{DH,DF,0,DP,DN},{DX,DV,0,Df,Dd},{
Dn,Dl,0,Dv,Dt},{C2,Cy,0,C_,C8},{D3,D1,0,EA,D9}};static CE*G4[2][2]={{Cu,Cw},{C4,
C6},};static CE*G5[6][2][2]={{{Cu,Cw},{C4,C6}},{{DB,DD},{DJ,DL}},{{DR,DT},{DZ,Db
}},{{Dh,Dj},{Dp,Dr}},{{Cu,Cw},{C4,C6}},{{Dx,Dz},{D5,D7}}};CA*G6=0;CE*G7=0;Bv G8,
G9;G8=Gx->Z;G9=Gx->a;Gw->C=Gx->B;Gw->E=Gx->R;Gw->F=Gx->S;if((Gw->A.G==//////////
STBIR_FILTER_POINT_SAMPLE)&&(Gw->B.G==STBIR_FILTER_POINT_SAMPLE))if(((G8==//////
STBIR_TYPE_UINT8_SRGB)||(G8==STBIR_TYPE_UINT8_SRGB_ALPHA))&&((G9==//////////////
STBIR_TYPE_UINT8_SRGB)||(G9==STBIR_TYPE_UINT8_SRGB_ALPHA))){G8=STBIR_TYPE_UINT8;
G9=STBIR_TYPE_UINT8;}if(Gw->E==0)Gw->E=Gw->d*Gw->A.E.A*B3[G8];if(Gw->F==0)Gw->F=
Gw->d*Gw->A.E.B*B3[G9];Gw->D=((char*)Gx->J)+((size_t)Gw->b*(size_t)Gx->S)+(Gw->a
*Gw->d*B3[G9]);Gw->K=Gx->I;Gw->L=Gx->A;Gw->M=Gx->Q;if((G8==STBIR_TYPE_UINT8)||(
G8==STBIR_TYPE_UINT16)){int G_=0;if((!Gw->R)&&(!Gw->T))if(((G8==STBIR_TYPE_UINT8
)&&(G9==STBIR_TYPE_UINT8))||((G8==STBIR_TYPE_UINT16)&&(G9==STBIR_TYPE_UINT16)))
G_=1;if(Gw->X<=STBIRI_4CHANNEL)G6=G0[G8==STBIR_TYPE_UINT16][G_];else G6=G1[(Gw->
X-STBIRI_RGBA)%6][G8==STBIR_TYPE_UINT16][G_];}else if(Gw->X<=STBIRI_4CHANNEL)G6=
Gy[G8-STBIR_TYPE_UINT8_SRGB];else G6=Gz[(Gw->X-STBIRI_RGBA)%6][G8-//////////////
STBIR_TYPE_UINT8_SRGB];if((G9==STBIR_TYPE_UINT8)||(G9==STBIR_TYPE_UINT16)){int//
G_=0;if((!Gw->R)&&(!Gw->T))if(((G8==STBIR_TYPE_UINT8)&&(G9==STBIR_TYPE_UINT8))||
((G8==STBIR_TYPE_UINT16)&&(G9==STBIR_TYPE_UINT16)))G_=1;if(Gw->Y<=//////////////
STBIRI_4CHANNEL)G7=G4[G9==STBIR_TYPE_UINT16][G_];else G7=G5[(Gw->Y-STBIRI_RGBA)%
6][G9==STBIR_TYPE_UINT16][G_];}else if(Gw->Y<=STBIRI_4CHANNEL)G7=G2[G9-/////////
STBIR_TYPE_UINT8_SRGB];else G7=G3[(Gw->Y-STBIRI_RGBA)%6][G9-////////////////////
STBIR_TYPE_UINT8_SRGB];Gw->I=G8;Gw->J=G9;Gw->Q=G6;Gw->U=G7;}static void Gq(int*
Gw,int*Gx,int Gy,double*Gz,double*G0){double G1,G2;int G3;if(*Gw<0){G1=((double)
*Gw)/((double)*Gx);G2=G1*(*G0-*Gz);*Gz-=G2;*Gw=0;}G3=Gy-(*Gw+*Gx);if(G3<0){G1=((
double)G3)/((double)*Gx);G2=G1*(*G0-*Gz);*G0+=G2;*Gx=Gy-*Gw;}}static int Gr(////
double Gw,Bq Gx,Bq*Gy,Bq*Gz,int G0){double G1;Br G2,G3;Br G4=0;Br G5=1;Br G6=1;
Br G7=0;G2=(Br)(Gw*33554432.0);G3=1<<25;for(;;){Br G8,G9;if((G0?G7:G6)>=Gx)break
;if(G7){G1=((double)G6/(double)G7)-Gw;if(G1<0.0)G1=-G1;if(G1<5.96e-08){*Gy=(Bq)
G6;*Gz=(Bq)G7;return 1;}}if(G3==0)break;G8=G2/G3;G9=G2%G3;G2=G3;G3=G9;G9=G8*G7+
G5;G5=G7;G7=G9;G9=G8*G6+G4;G4=G6;G6=G9;}if(G0){G6=(Br)(Gw*(double)Gx+0.5);G7=Gx;
}else{G6=Gx;G7=(Br)(((double)Gx/Gw)+0.5);}*Gy=(Bq)G6;*Gz=(Bq)G7;G1=G7?(((double)
(Bq)G6/(double)(Bq)G7)-Gw):1.0;if(G1<0.0)G1=-G1;return(G1<5.96e-08)?1:0;}static
int Gs(B7*Gw,int Gx,int*Gy,int Gz,int G0,double G1,double G2){double G3,G4,G5,G6
,G7,G8;G6=G2-G1;if((Gx==0)||(G0==0)||(Gz==0)||(G6<=7.523e-37))return 0;if((*Gy>=
Gx)||((*Gy+Gz)<=0)||(G1>=1.0)||(G2<=7.523e-37))return 0;G3=(double)Gx;G4=(double
)G0;G5=((double)Gz)/G3;G7=G5/G6;G8=(G3/G4)*G7;Gw->C=(Z1)G8;Gw->D=(Z1)(1.0/G8);Gq
(Gy,&Gz,Gx,&G1,&G2);G6=G2-G1;if(G6<=7.523e-37)return 0;Gw->E=(Z1)(G1*G7*G3);Gw->
F=Gr(G8,(G8<=1.0)?Gx:G0,&Gw->G,&Gw->H,G8>=1.0);Gw->A=G0;Gw->B=Gz;return 1;}/////
static void Gt(B1*Gw,Bs Gx,Bv Gy){Gw->I=0;Gw->Q=0;Gw->A=Gw;Gw->j=0;Gw->W=0;Gw->b
=STBIR_FILTER_DEFAULT;Gw->f=0;Gw->g=0;Gw->c=STBIR_FILTER_DEFAULT;Gw->h=0;Gw->i=0
;Gw->d=STBIR_EDGE_CLAMP;Gw->e=STBIR_EDGE_CLAMP;Gw->E=0;Gw->F=0;Gw->G=1;Gw->H=1;
Gw->M=0;Gw->N=0;Gw->O=Gw->K;Gw->P=Gw->L;Gw->Z=Gy;Gw->a=Gy;Gw->X=Gx;Gw->Y=Gx;Gw->
V=1;}extern void stbir_resize_init(B1*Gw,const void*Gx,int Gy,int Gz,int G0,void
*G1,int G2,int G3,int G4,Bs G5,Bv G6){Gw->B=Gx;Gw->C=Gy;Gw->D=Gz;Gw->R=G0;Gw->J=
G1;Gw->K=G2;Gw->L=G3;Gw->S=G4;Gw->U=0;Gt(Gw,G5,G6);}extern void/////////////////
stbir_set_datatypes(B1*Gw,Bv Gx,Bv Gy){Gw->Z=Gx;Gw->a=Gy;if(Gw->j&&(!Gw->V))Gp(
Gw->j,Gw);}extern void stbir_set_pixel_callbacks(B1*Gw,Bw*Gx,Bx*Gy){Gw->I=Gx;Gw
->Q=Gy;if(Gw->j&&(!Gw->V)){Gw->j->K=Gx;Gw->j->M=Gy;}}extern void////////////////
stbir_set_user_data(B1*Gw,void*Gx){Gw->A=Gx;if(Gw->j&&(!Gw->V))Gw->j->L=Gx;}////
extern void stbir_set_buffer_ptrs(B1*Gw,const void*Gx,int Gy,void*Gz,int G0){Gw
->B=Gx;Gw->R=Gy;Gw->J=Gz;Gw->S=G0;if(Gw->j&&(!Gw->V))Gp(Gw->j,Gw);}extern int///
stbir_set_edgemodes(B1*Gw,Bt Gx,Bt Gy){Gw->d=Gx;Gw->e=Gy;Gw->V=1;return 1;}/////
extern int stbir_set_filters(B1*Gw,Bu Gx,Bu Gy){Gw->b=Gx;Gw->c=Gy;Gw->V=1;return
1;}extern int stbir_set_filter_callbacks(B1*Gw,By*Gx,Bz*Gy,By*Gz,Bz*G0){Gw->f=Gx
;Gw->g=Gy;Gw->h=Gz;Gw->i=G0;Gw->V=1;return 1;}extern int stbir_set_pixel_layouts
(B1*Gw,Bs Gx,Bs Gy){Gw->X=Gx;Gw->Y=Gy;Gw->V=1;return 1;}extern int//////////////
stbir_set_non_pm_alpha_speed_over_quality(B1*Gw,int Gx){Gw->U=Gx;Gw->V=1;return
1;}extern int stbir_set_input_subrect(B1*Gw,double Gx,double Gy,double Gz,double
G0){Gw->E=Gx;Gw->F=Gy;Gw->G=Gz;Gw->H=G0;Gw->V=1;if((Gz<7.523e-37)||((Gz-Gx)<////
7.523e-37)||(G0<7.523e-37)||((G0-Gy)<7.523e-37)||(Gx>1.0)||(Gy>1.0))return 0;///
return 1;}extern int stbir_set_output_pixel_subrect(B1*Gw,int Gx,int Gy,int Gz,
int G0){Gw->M=Gx;Gw->N=Gy;Gw->O=Gz;Gw->P=G0;Gw->V=1;if((Gx>=Gw->K)||((Gx+Gz)<=0)
||(Gy>=Gw->L)||((Gy+G0)<=0)||(Gz==0)||(G0==0))return 0;return 1;}extern int/////
stbir_set_pixel_subrect(B1*Gw,int Gx,int Gy,int Gz,int G0){double G1,G2,G3,G4;G1
=((double)Gx)/((double)Gw->K);G2=((double)Gy)/((double)Gw->L);G3=((double)(Gx+Gz
))/((double)Gw->K);G4=((double)(Gy+G0))/((double)Gw->L);Gw->E=G1;Gw->F=G2;Gw->G=
G3;Gw->H=G4;Gw->M=Gx;Gw->N=Gy;Gw->O=Gz;Gw->P=G0;Gw->V=1;if((Gx>=Gw->K)||((Gx+Gz)
<=0)||(Gy>=Gw->L)||((Gy+G0)<=0)||(Gz==0)||(G0==0))return 0;return 1;}static int
Gu(B1*Gw,int Gx){B4 Gy={0,0};B8 Gz,G0;int G1,G2;B0*G3;if(Gw->j)return 0;if((////
unsigned)Gw->b>=STBIR_FILTER_OTHER)return 0;if((unsigned)Gw->c>=////////////////
STBIR_FILTER_OTHER)return 0;if(Gx<=0)return 0;G1=Gw->M;G2=Gw->N;if(!Gs(&Gz.E,Gw
->K,&G1,Gw->O,Gw->C,Gw->E,Gw->G))return 0;if(!Gs(&G0.E,Gw->L,&G2,Gw->P,Gw->D,Gw
->F,Gw->H))return 0;if((Gz.E.B==0)||(G0.E.B==0))return 0;Gb(&Gz,Gw->b,Gw->f,Gw->
g,Gw->d,&Gz.E,1,Gw->A);Gc(&Gz,&Gy,Gw->A);Gb(&G0,Gw->c,Gw->h,Gw->i,Gw->e,&G0.E,0,
Gw->A);if((G0.E.B/Gx)<4){Gx=G0.E.B/4;if(Gx==0)Gx=1;}G3=Gn(&Gz,&G0,&Gy,Gw->X,Gw->
Y,Gx,G1,G2,Gw->U,Gw->A);if(G3){Gw->T=Gx;Gw->j=G3;Gw->V=0;Gp(G3,Gw);return Gx;}//
return 0;}extern void stbir_free_samplers(B1*Gw){if(Gw->j){Ge(Gw->j);Gw->j=0;Gw
->W=0;}}extern int stbir_build_samplers_with_splits(B1*Gw,int Gx){if((Gw->j==0)
||Gw->V){if(Gw->j)stbir_free_samplers(Gw);Gw->W=1;return Gu(Gw,Gx);}return 1;}//
extern int stbir_build_samplers(B1*Gw){return stbir_build_samplers_with_splits(
Gw,1);}extern int stbir_resize_extended(B1*Gw){int Gx;if((Gw->j==0)||Gw->V){int
Gy=Gw->W;if(Gw->j){Ge(Gw->j);Gw->j=0;}if(!stbir_build_samplers(Gw))return 0;Gw->
W=Gy;if(Gw->j==0)return 1;}else{}Gx=Go(Gw->j,0,Gw->T);if(!Gw->W){///////////////
stbir_free_samplers(Gw);Gw->j=0;}return Gx;}extern int//////////////////////////
stbir_resize_extended_split(B1*Gw,int Gx,int Gy){if((Gx==-1)||((Gx==0)&&(Gy==Gw
->T)))return stbir_resize_extended(Gw);if((Gw->j==0)||Gw->V)return 0;if((Gx>=Gw
->T)||(Gx<0)||((Gx+Gy)>Gw->T)||(Gy<=0))return 0;return Go(Gw->j,Gx,Gy);}static//
void*Gv(const void*Gw,int Gx,int Gy,int Gz,void*G0,int G1,int G2,int G3,Bs G4,Bv
G5,Bt G6,Bu G7){B1 G8;int G9;int G_;void*HA;void*HB;G9=G1*B3[G5]*Gl[Gm[G4]];if(
G9==0)return 0;if(G3==0)G3=G9;G_=G3;if(G_<0)G_=-G_;if(G_<G9)return 0;HA=G0;HB=0;
if(G0==0){size_t HC;char*HD;HC=(size_t)G_*(size_t)G2;if(HC==0)return 0;HD=(char*
)((void)0,malloc(HC));if(HD==0)return 0;HB=HD;if(G3<0)HA=HD+((size_t)G_*(size_t)
(G2-1));else HA=HD;}stbir_resize_init(&G8,Gw,Gx,Gy,Gz,HA,G1,G2,G3,G4,G5);G8.d=G6
;G8.e=G6;G8.b=G7;G8.c=G7;if(!stbir_resize_extended(&G8)){if(HB)(void)0,free(HB);
return 0;}return HB?HB:HA;}extern unsigned char*stbir_resize_uint8_linear(const
unsigned char*Gw,int Gx,int Gy,int Gz,unsigned char*G0,int G1,int G2,int G3,Bs//
G4){return(unsigned char*)Gv(Gw,Gx,Gy,Gz,G0,G1,G2,G3,G4,STBIR_TYPE_UINT8,///////
STBIR_EDGE_CLAMP,STBIR_FILTER_DEFAULT);}extern unsigned char*///////////////////
stbir_resize_uint8_srgb(const unsigned char*Gw,int Gx,int Gy,int Gz,unsigned////
char*G0,int G1,int G2,int G3,Bs G4){return(unsigned char*)Gv(Gw,Gx,Gy,Gz,G0,G1,
G2,G3,G4,STBIR_TYPE_UINT8_SRGB,STBIR_EDGE_CLAMP,STBIR_FILTER_DEFAULT);}extern Z1
*stbir_resize_Z1_linear(const Z1*Gw,int Gx,int Gy,int Gz,Z1*G0,int G1,int G2,int
G3,Bs G4){return(Z1*)Gv(Gw,Gx,Gy,Gz,G0,G1,G2,G3,G4,STBIR_TYPE_Z1,///////////////
STBIR_EDGE_CLAMP,STBIR_FILTER_DEFAULT);}extern void*stbir_resize(const void*Gw,
int Gx,int Gy,int Gz,void*G0,int G1,int G2,int G3,Bs G4,Bv G5,Bt G6,Bu G7){/////
return(void*)Gv(Gw,Gx,Gy,Gz,G0,G1,G2,G3,G4,G5,G6,G7);}//////////////////////////

#if defined(__MSVC__)
#  pragma warning(pop)
#else
#  pragma GCC diagnostic pop
#endif
// clang-format on
// NOLINTEND
