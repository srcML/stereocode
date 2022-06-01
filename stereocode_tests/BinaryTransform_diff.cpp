/*
/transforms files differences

BinaryTransform.cxx / hpp
ATTRIBUTES: 
TYPE: UnaryTransform NAME: m_z
TYPE: bool NAME: m_needs_grid
TYPE: bool NAME: m_needs_x_ticks
TYPE: bool NAME: m_needs_y_ticks
TYPE: bool NAME: m_is_periodic

7/13 stereotypes were the same
*/


/** @stereotype collaborational-property pure_stateless */ //property
double BinaryTransform::aspectRatio () const
{
  return 0.0;
}

/** @stereotype get collaborator */ //nothing yet
TransformBase * BinaryTransform::zTransform () const
{
  return m_z;
}

/** @stereotype voidaccessor */ //Controller
void BinaryTransform::transformZ ( double & z ) const
{
  assert ( m_z );
  m_z->transform ( z );
}

/** @stereotype voidaccessor */ //Controller
void BinaryTransform::inverseTransformZ ( double & z ) const
{
  assert ( m_z );
  m_z->inverseTransform ( z );
}

/** @stereotype property collaborator */ //Controller
const Range & BinaryTransform::limitZ () const
{
  assert ( m_z );
  return m_z->limits();
}
