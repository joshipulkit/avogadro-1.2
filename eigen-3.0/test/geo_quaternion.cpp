// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2009 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2009 Mathieu Gautier <mathieu.gautier@cea.fr>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#include "main.h"
#include <Eigen/Geometry>
#include <Eigen/LU>
#include <Eigen/SVD>

template<typename T> T bounded_acos(T v)
{
  using std::acos;
  using std::min;
  using std::max;
  return acos((max)(T(-1),(min)(v,T(1))));
}

template<typename QuatType> void check_slerp(const QuatType& q0, const QuatType& q1)
{
  typedef typename QuatType::Scalar Scalar;
  typedef Matrix<Scalar,3,1> VectorType;
  typedef AngleAxis<Scalar> AA;

  Scalar largeEps = test_precision<Scalar>();

  Scalar theta_tot = AA(q1*q0.inverse()).angle();
  if(theta_tot>M_PI)
    theta_tot = 2.*M_PI-theta_tot;
  for(Scalar t=0; t<=1.001; t+=0.1)
  {
    QuatType q = q0.slerp(t,q1);
    Scalar theta = AA(q*q0.inverse()).angle();
    VERIFY(internal::abs(q.norm() - 1) < largeEps);
    if(theta_tot==0)  VERIFY(theta_tot==0);
    else              VERIFY(internal::abs(theta/theta_tot - t) < largeEps);
  }
}

template<typename Scalar, int Options> void quaternion(void)
{
  /* this test covers the following files:
     Quaternion.h
  */

  typedef Matrix<Scalar,3,3> Matrix3;
  typedef Matrix<Scalar,3,1> Vector3;
  typedef Matrix<Scalar,4,1> Vector4;
  typedef Quaternion<Scalar,Options> Quaternionx;
  typedef AngleAxis<Scalar> AngleAxisx;

  Scalar largeEps = test_precision<Scalar>();
  if (internal::is_same<Scalar,float>::value)
    largeEps = 1e-3f;

  Scalar eps = internal::random<Scalar>() * Scalar(1e-2);

  Vector3 v0 = Vector3::Random(),
          v1 = Vector3::Random(),
          v2 = Vector3::Random(),
          v3 = Vector3::Random();

  Scalar  a = internal::random<Scalar>(-Scalar(M_PI), Scalar(M_PI)),
          b = internal::random<Scalar>(-Scalar(M_PI), Scalar(M_PI));

  // Quaternion: Identity(), setIdentity();
  Quaternionx q1, q2;
  q2.setIdentity();
  VERIFY_IS_APPROX(Quaternionx(Quaternionx::Identity()).coeffs(), q2.coeffs());
  q1.coeffs().setRandom();
  VERIFY_IS_APPROX(q1.coeffs(), (q1*q2).coeffs());

  // concatenation
  q1 *= q2;

  q1 = AngleAxisx(a, v0.normalized());
  q2 = AngleAxisx(a, v1.normalized());

  // angular distance
  Scalar refangle = internal::abs(AngleAxisx(q1.inverse()*q2).angle());
  if (refangle>Scalar(M_PI))
    refangle = Scalar(2)*Scalar(M_PI) - refangle;

  if((q1.coeffs()-q2.coeffs()).norm() > 10*largeEps)
  {
    VERIFY_IS_MUCH_SMALLER_THAN(internal::abs(q1.angularDistance(q2) - refangle), Scalar(1));
  }

  // rotation matrix conversion
  VERIFY_IS_APPROX(q1 * v2, q1.toRotationMatrix() * v2);
  VERIFY_IS_APPROX(q1 * q2 * v2,
    q1.toRotationMatrix() * q2.toRotationMatrix() * v2);

  VERIFY(  (q2*q1).isApprox(q1*q2, largeEps)
        || !(q2 * q1 * v2).isApprox(q1.toRotationMatrix() * q2.toRotationMatrix() * v2));

  q2 = q1.toRotationMatrix();
  VERIFY_IS_APPROX(q1*v1,q2*v1);


  // angle-axis conversion
  AngleAxisx aa = AngleAxisx(q1);
  VERIFY_IS_APPROX(q1 * v1, Quaternionx(aa) * v1);

  // Do not execute the test if the rotation angle is almost zero, or
  // the rotation axis and v1 are almost parallel.
  if (internal::abs(aa.angle()) > 5*test_precision<Scalar>()
      && (aa.axis() - v1.normalized()).norm() < 1.99
      && (aa.axis() + v1.normalized()).norm() < 1.99) 
  {
    VERIFY_IS_NOT_APPROX(q1 * v1, Quaternionx(AngleAxisx(aa.angle()*2,aa.axis())) * v1);
  }

  // from two vector creation
  VERIFY_IS_APPROX( v2.normalized(),(q2.setFromTwoVectors(v1, v2)*v1).normalized());
  VERIFY_IS_APPROX( v1.normalized(),(q2.setFromTwoVectors(v1, v1)*v1).normalized());
  VERIFY_IS_APPROX(-v1.normalized(),(q2.setFromTwoVectors(v1,-v1)*v1).normalized());
  if (internal::is_same<Scalar,double>::value)
  {
    v3 = (v1.array()+eps).matrix();
    VERIFY_IS_APPROX( v3.normalized(),(q2.setFromTwoVectors(v1, v3)*v1).normalized());
    VERIFY_IS_APPROX(-v3.normalized(),(q2.setFromTwoVectors(v1,-v3)*v1).normalized());
  }

  // inverse and conjugate
  VERIFY_IS_APPROX(q1 * (q1.inverse() * v1), v1);
  VERIFY_IS_APPROX(q1 * (q1.conjugate() * v1), v1);

  // test casting
  Quaternion<float> q1f = q1.template cast<float>();
  VERIFY_IS_APPROX(q1f.template cast<Scalar>(),q1);
  Quaternion<double> q1d = q1.template cast<double>();
  VERIFY_IS_APPROX(q1d.template cast<Scalar>(),q1);

  // test bug 369 - improper alignment.
  Quaternionx *q = new Quaternionx;
  delete q;

  q1 = AngleAxisx(a, v0.normalized());
  q2 = AngleAxisx(b, v1.normalized());
  check_slerp(q1,q2);

  q1 = AngleAxisx(b, v1.normalized());
  q2 = AngleAxisx(b+M_PI, v1.normalized());
  check_slerp(q1,q2);

  q1 = AngleAxisx(b,  v1.normalized());
  q2 = AngleAxisx(-b, -v1.normalized());
  check_slerp(q1,q2);

  q1.coeffs() = Vector4::Random().normalized();
  q2.coeffs() = -q1.coeffs();
  check_slerp(q1,q2);
}

template<typename Scalar> void mapQuaternion(void){
  typedef Map<Quaternion<Scalar>, Aligned> MQuaternionA;
  typedef Map<const Quaternion<Scalar>, Aligned> MCQuaternionA;
  typedef Map<Quaternion<Scalar> > MQuaternionUA;
  typedef Map<const Quaternion<Scalar> > MCQuaternionUA;
  typedef Quaternion<Scalar> Quaternionx;
  typedef Matrix<Scalar,3,1> Vector3;
  typedef AngleAxis<Scalar> AngleAxisx;
  
  Vector3 v0 = Vector3::Random(),
          v1 = Vector3::Random();
  Scalar  a = internal::random<Scalar>(-Scalar(M_PI), Scalar(M_PI));

  EIGEN_ALIGN16 Scalar array1[4];
  EIGEN_ALIGN16 Scalar array2[4];
  EIGEN_ALIGN16 Scalar array3[4+1];
  Scalar* array3unaligned = array3+1;
  
  MQuaternionA    mq1(array1);
  MCQuaternionA   mcq1(array1);
  MQuaternionA    mq2(array2);
  MQuaternionUA   mq3(array3unaligned);
  MCQuaternionUA  mcq3(array3unaligned);

//  std::cerr << array1 << " " << array2 << " " << array3 << "\n";
  mq1 = AngleAxisx(a, v0.normalized());
  mq2 = mq1;
  mq3 = mq1;

  Quaternionx q1 = mq1;
  Quaternionx q2 = mq2;
  Quaternionx q3 = mq3;
  Quaternionx q4 = MCQuaternionUA(array3unaligned);

  VERIFY_IS_APPROX(q1.coeffs(), q2.coeffs());
  VERIFY_IS_APPROX(q1.coeffs(), q3.coeffs());
  VERIFY_IS_APPROX(q4.coeffs(), q3.coeffs());
  #ifdef EIGEN_VECTORIZE
  if(internal::packet_traits<Scalar>::Vectorizable)
    VERIFY_RAISES_ASSERT((MQuaternionA(array3unaligned)));
  #endif
    
  VERIFY_IS_APPROX(mq1 * (mq1.inverse() * v1), v1);
  VERIFY_IS_APPROX(mq1 * (mq1.conjugate() * v1), v1);
  
  VERIFY_IS_APPROX(mcq1 * (mcq1.inverse() * v1), v1);
  VERIFY_IS_APPROX(mcq1 * (mcq1.conjugate() * v1), v1);
  
  VERIFY_IS_APPROX(mq3 * (mq3.inverse() * v1), v1);
  VERIFY_IS_APPROX(mq3 * (mq3.conjugate() * v1), v1);
  
  VERIFY_IS_APPROX(mcq3 * (mcq3.inverse() * v1), v1);
  VERIFY_IS_APPROX(mcq3 * (mcq3.conjugate() * v1), v1);
  
  VERIFY_IS_APPROX(mq1*mq2, q1*q2);
  VERIFY_IS_APPROX(mq3*mq2, q3*q2);
  VERIFY_IS_APPROX(mcq1*mq2, q1*q2);
  VERIFY_IS_APPROX(mcq3*mq2, q3*q2);
}

template<typename Scalar> void quaternionAlignment(void){
  typedef Quaternion<Scalar,AutoAlign> QuaternionA;
  typedef Quaternion<Scalar,DontAlign> QuaternionUA;

  EIGEN_ALIGN16 Scalar array1[4];
  EIGEN_ALIGN16 Scalar array2[4];
  EIGEN_ALIGN16 Scalar array3[4+1];
  Scalar* arrayunaligned = array3+1;

  QuaternionA *q1 = ::new(reinterpret_cast<void*>(array1)) QuaternionA;
  QuaternionUA *q2 = ::new(reinterpret_cast<void*>(array2)) QuaternionUA;
  QuaternionUA *q3 = ::new(reinterpret_cast<void*>(arrayunaligned)) QuaternionUA;

  q1->coeffs().setRandom();
  *q2 = *q1;
  *q3 = *q1;

  VERIFY_IS_APPROX(q1->coeffs(), q2->coeffs());
  VERIFY_IS_APPROX(q1->coeffs(), q3->coeffs());
  #if defined(EIGEN_VECTORIZE) && EIGEN_ALIGN_STATICALLY
  if(internal::packet_traits<Scalar>::Vectorizable)
    VERIFY_RAISES_ASSERT((::new(reinterpret_cast<void*>(arrayunaligned)) QuaternionA));
  #endif
}

template<typename PlainObjectType> void check_const_correctness(const PlainObjectType&)
{
  // there's a lot that we can't test here while still having this test compile!
  // the only possible approach would be to run a script trying to compile stuff and checking that it fails.
  // CMake can help with that.

  // verify that map-to-const don't have LvalueBit
  typedef typename internal::add_const<PlainObjectType>::type ConstPlainObjectType;
  VERIFY( !(internal::traits<Map<ConstPlainObjectType> >::Flags & LvalueBit) );
  VERIFY( !(internal::traits<Map<ConstPlainObjectType, Aligned> >::Flags & LvalueBit) );
  VERIFY( !(Map<ConstPlainObjectType>::Flags & LvalueBit) );
  VERIFY( !(Map<ConstPlainObjectType, Aligned>::Flags & LvalueBit) );
}

void test_geo_quaternion()
{
  for(int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_1(( quaternion<float,AutoAlign>() ));
    CALL_SUBTEST_1( check_const_correctness(Quaternionf()) );
    CALL_SUBTEST_2(( quaternion<double,AutoAlign>() ));
    CALL_SUBTEST_2( check_const_correctness(Quaterniond()) );
    CALL_SUBTEST_3(( quaternion<float,DontAlign>() ));
    CALL_SUBTEST_4(( quaternion<double,DontAlign>() ));
    CALL_SUBTEST_5(( quaternionAlignment<float>() ));
    CALL_SUBTEST_6(( quaternionAlignment<double>() ));
    CALL_SUBTEST_1( mapQuaternion<float>() );
    CALL_SUBTEST_2( mapQuaternion<double>() );
  }
}
