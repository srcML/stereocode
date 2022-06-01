/*
PeriodicBinaryTransform.cxx / hpp
ATTRIBUTES: 
TYPE: Range NAME: m_x_limits
TYPE: Range NAME: m_y_limits
TYPE: double NAME: m_x_offset
TYPE: double NAME: m_y_offset

6/12 same as old tool
*/

/** @stereotype get collaborator */ //Nothing yet - 2 methods similar
const Range & PeriodicBinaryTransform::limitY () const
{
  return m_y_limits;
}

/** @stereotype property */  //Get - 4 methods very similar.
double PeriodicBinaryTransform::moduloSubY( double y1, double y2 ) const
{
  if ( y2 < -DBL_EPSILON ) return moduloAddY ( y1, -y2 );
  
  double undershoot = m_y_limits.low() - ( y1 - y2 );
    
  if ( undershoot > DBL_EPSILON )  return  m_y_limits.high() - undershoot;
  else return y1 - y2;
}