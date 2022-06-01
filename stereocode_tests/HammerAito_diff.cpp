/*
HammerAito.cxx / hpp
ATTRIBUTES: 
TYPE: std::vector < AxisTick > NAME: m_ticks
*/
/** @stereotype collaborational-predicate pure_stateless */  //Not recognized as a method.
bool
HammerAito::
isLinearInXY () const
{
  return false;
}

/* virtual */
/** @stereotype voidaccessor */ //Not recognized as a method.
void
HammerAito::
transform ( double & lon, double & lat ) const
{
  // Note we need not take care of offset inside this function because
  // all the points given to this function are implicitly assumed to
  // be such that they have been altered to take care of the offset
   
  double lon_r_2 = M_PI * lon / 360.0; // to radians and divide by 2.
  double lat_r = M_PI * lat / 180.0; // to radians

  // Transformation and conversion back to degrees are combined in these steps
  double cos_lat = cos ( lat_r );
  double t = sqrt ( 2.0 / ( 1.0 + cos_lat * cos ( lon_r_2 ) ) );

  lon = ( 180.0 / M_PI ) * 2.0 * t * cos_lat * sin ( lon_r_2 );
  lat = ( 180.0 / M_PI ) * t * sin ( lat_r );
}

/** @stereotype voidaccessor */ //Not recognized as a method.
void
HammerAito::
inverseTransform ( double & lon, double & lat ) const
{
  // Note we need not take care of offset inside this function because
  // all the points returned by this function are implicitly assumed to
  // be processed such that they get altered to take care of the offset
  
  double lon_r = lon * M_PI / 180.0; // to radians
  double lat_r = lat * M_PI / 180.0; // to radians

  double lon_q = 0.25 * lon_r;
  double lat_h = 0.50 * lat_r;

  // Inversion and conversion back to degrees are combined in these steps
  double z = sqrt ( 1.0 - lon_q * lon_q - lat_h * lat_h );

  lon = ( 180.0 / M_PI ) * 2.0 * atan ( ( z * lon_r ) / 
					( 4.0 * z * z - 2.0 ) );
  lat = ( 180.0 / M_PI ) * asin ( lat_r * z );
}

/* virtual */
/** @stereotype voidaccessor */ //Not recognized as a method.
void HammerAito::transform ( vector< double > & lon,
			     vector< double > & lat ) const
{
  assert ( lat.size() == lon.size() );

  std::vector < double >:: iterator it_lat = lat.begin ();
  std::vector < double >:: iterator it_lon = lon.begin ();

  for ( ; it_lat != lat.end(); ++it_lat, ++it_lon ) {
    transform ( *it_lon, *it_lat );
  }
}

/* virtual */
/** @stereotype collaborational-property pure_stateless */ //Not recognized as a method.
double HammerAito::aspectRatio () const
{
  return 2.0;
}

/** @stereotype non-void-command collaborator */ //Not recognized as a method
HippoRectangle HammerAito::calcRectangle ( const Range & lat,
					   const Range & lon )
{
  double x_lo = lat.low ();
  double x_hi = lat.high ();
  
  double y_lo = lon.low ();
  double y_hi = lon.high ();

  double x, y;
  
  double x_min =  1000;
  double x_max = -1000;
  double y_min =  1000;
  double y_max = -1000;

  int n = 50;
  double dx = ( x_hi - x_lo ) / n;
  double dy = ( y_hi - y_lo ) / n;

  
  // Finding out xmin, xmax, ymin, ymax along line y =  y_lo
  for ( int i = 0; i <= n; i++)
    {
      x = x_lo + i * dx; 
      y = y_lo;
      transform ( x, y );
      x_min = ( x_min < x ) ? x_min : x;
      x_max = ( x_max > x ) ? x_max : x;
      y_min = ( y_min < y ) ? y_min : y;
      y_max = ( y_max > y ) ? y_max : y;
    }

  // Finding out xmin, xmax, ymin, ymax along line y =  y_hi
  for ( int i = 0; i <= n; i++)
    {
      x = x_lo + i * dx; 
      y = y_hi;
      transform ( x, y );
      x_min = ( x_min < x ) ? x_min : x;
      x_max = ( x_max > x ) ? x_max : x;
      y_min = ( y_min < y ) ? y_min : y;
      y_max = ( y_max > y ) ? y_max : y;
    }

  // Finding out xmin, xmax, ymin, ymax along line x =  x_lo
  for ( int i = 0; i <= n; i++)
    {
      x = x_lo; 
      y = y_lo + i * dy;
      transform ( x, y );
      x_min = ( x_min < x ) ? x_min : x;
      x_max = ( x_max > x ) ? x_max : x;
      y_min = ( y_min < y ) ? y_min : y;
      y_max = ( y_max > y ) ? y_max : y;
    }

  // Finding out xmin, xmax, ymin, ymax along line x =  x_hi
  for ( int i = 0; i <= n; i++)
    {
      x = x_hi; 
      y = y_lo + i * dy;
      transform ( x, y );
      x_min = ( x_min < x ) ? x_min : x;
      x_max = ( x_max > x ) ? x_max : x;
      y_min = ( y_min < y ) ? y_min : y;
      y_max = ( y_max > y ) ? y_max : y;
    }
    
  return HippoRectangle ( x_min, y_min, x_max - x_min, y_max - y_min );
}


/* virtual */
/** @stereotype collaborational-voidaccessor collaborator */ //Not recognized as a method.
void HammerAito::validate ( Range & lat, Range & lon ) const
{
  if ( lat.low () < -180.0 ) lat.setLow ( -180.0 );
  if ( lat.high () > 180.0 ) lat.setHigh ( 180.0 );

  if ( lon.low () < -90.0 ) lon.setLow ( -90.0 );
  if ( lon.high () > 90.0 ) lon.setHigh ( 90.0 );
}


/** @stereotype non-void-command collaborator */  //Controller
const vector < AxisTick > &
HammerAito::
setTicks ( AxisModelBase & model, Axes::Type axis )
{
  setTickStep ( model );
  setFirstTick ( model );
  return genTicks ( model, axis );
}

/** @stereotype collaborator pure_stateless */ //Controller
void
HammerAito::
adjustValues ( AxisModelBase & model,
         Axes::Type,
         const Range & limit )
{
  // Does not do anything as of now 
  return;
}

/** @stereotype command collaborator */ //Controller
void HammerAito::setTickStep( AxisModelBase & axis )
{
  const Range & range = axis.getRange(false);
  double rangeLength = range.length();

  double scale_factor = axis.getScaleFactor();
  rangeLength *= scale_factor;
  
  // The following algorithm determines the magnitude of the range...
  double rmag = floor( log10( rangeLength ) );
  axis.setRMag( rmag );
  
  double scalelow = range.low() * scale_factor;
  double scalehigh = range.high() * scale_factor;

  // We will also need the largest magnitude for labels.
  double pmag = max( floor( log10( abs ( scalehigh ) ) ), 
         floor( log10( abs ( scalelow ) ) ) );
  axis.setPMag( pmag );
  
  axis.setTickStep( rangeLength / 4.0 );
}

/** @stereotype collaborational-command collaborator */ //Controller
void HammerAito::setFirstTick( AxisModelBase & axis )
{
  const Range & range = axis.getRange(false);
  
  axis.setFirstTick ( range.low() );
}


/** @stereotype non-void-command collaborator */ //Controller
const vector < AxisTick > &
HammerAito::
genTicks( AxisModelBase & axis, Axes::Type axistype )
{ 
  double y = 0.0, ylabel;
  
  int num_ticks = 0;
  m_ticks.clear();
  double pmag = axis.getPMag();
  double rmag = axis.getRMag();
  double first_tick = axis.getFirstTick();
  double tick_step  = axis.getTickStep();
  double scale_factor = axis.getScaleFactor();
  
  // pmag will get set to 0 if it is less than or equal to 3.  This
  // is used later to determine scientific notation.  However, m_rmag
  // is still needed as the original magnitude for calculations such
  // as decimal place notation, and rounding to nice numbers.
  
  bool use_pmag = abs ( pmag ) > 3.0;

  axis.setUsePMag ( use_pmag );
  
  char pstr[10];
  char labl[10];

  int decimals = 0;

  // The following if-else block decides the pstr string, which holds
  // the number of decimal places in the label.

  //   if( fabs( m_pmag ) > 3.0 ) {
  if ( use_pmag ) {  
    // If we are greater than mag 3, we are showing scientific
    // notation.  How many decimals we show is determined by the
    // difference between the range magnitude and the power magnitude.
  
    decimals = static_cast<int>( pmag - rmag );
    // minumum 1 decimal in scientific notation
    
    if( !decimals ) decimals++;
  
  } else {
    
    if( rmag > 0.0 ){
    
      // If we are less than mag 3 and positive, then no decimal
      // accuracy is needed.
      
      decimals = 0;
      
    } else {
    
      // If we are less than mag 3 and negative, then we are suddenly
      // looking at decimal numbers not in scientific notation.
      // Therefore we hold as many decimal places as the magnitude.
      
      decimals = static_cast<int>( abs( rmag ) );
      
    }
  
  }
  // @todo decimals should never be negative here, but it does end up
  //    negative in some cases. See the "dirty fix" in Range.cxx, that
  //    dirty-fixed this problem too. But a better fix is needed. 
  if (decimals < 0) 
    decimals = 0;
   
  sprintf( pstr, "%%1.%df", decimals );

  y = first_tick;
  const Range & range = axis.getRange(false);
  double range_high = range.high();
  range_high *= scale_factor;
  
  while( y <= range_high || FLT_EQUAL( range_high, y ) )
    {
      double value = 0.0;
      
      if ( axistype == Axes::X ) {
  value = moduloAddX( y, xOffset() );
      }
      else if ( axistype == Axes::Y ) {
  value = moduloAddY( y, yOffset() );
      }
      
      // Now we either keep the original magnitude
      // or reduce it in order to express it in scientific notation.
      
      if ( use_pmag ) ylabel = value / pow( 10.0, pmag );
      else ylabel = value;
      
      value /= scale_factor;
      sprintf( labl, pstr, ylabel );
      m_ticks.push_back( AxisTick ( y, labl ) );
      
      num_ticks++;
      y += tick_step;
      
    }
  
  return m_ticks;
}
