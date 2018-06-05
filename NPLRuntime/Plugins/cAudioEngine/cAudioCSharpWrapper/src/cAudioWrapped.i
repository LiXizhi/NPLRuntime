%module(directors="1") cAudioCSharpWrapper
%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%include "cpointer.i"

#define CAUDIO_API

typedef basic_string<char> cAudioString;

%{
#include "cAudioString.h"
#include "IRefCounted.h"
#include "EAudioFormats.h"
#include "IAudioEffects.h"
#include "IAudioManager.h"
#include "IAudioPlugin.h"
#include "IAudioSource.h"
#include "ICaptureEventHandler.h"
#include "IDataSource.h"
#include "IDataSourceFactory.h"
#include "IEffect.h"
#include "IEffectParameters.h"
#include "IFilter.h"
#include "IListener.h"
#include "ILogger.h"
#include "ILogReceiver.h"
#include "IManagerEventHandler.h"
#include "IMemoryProvider.h"
#include "IPluginManager.h"
#include "ISourceEventHandler.h"
#include "IThread.h"
#include "cAudio.h"
#include "cAudioDefines.h"
#include "cAudioMemory.h"
#include "cAudioPlatform.h"
#include "cAudioSleep.h"
#include "cSTLAllocator.h"
#include "cVector3.h"
#include "IAudioCapture.h"
#include "IAudioDecoder.h"
#include "IAudioDecoderFactory.h"
#include "IAudioDeviceList.h"
#include "IAudioDeviceContext.h"
%}

#if !defined(SWIGLUA) && !defined(SWIGR)
%rename(Equal) operator =;
%rename(PlusEqual) operator +=;
%rename(MinusEqual) operator -=;
%rename(MultiplyEqual) operator *=;
%rename(DivideEqual) operator /=;
%rename(PercentEqual) operator %=;
%rename(Plus) operator +;
%rename(Minus) operator -;
%rename(Multiply) operator *;
%rename(Divide) operator /;
%rename(Percent) operator %;
%rename(Not) operator !;
%rename(IndexIntoConst) operator[](size_t) const;
%rename(IndexIntoConstConst) operator [] ( const size_t ) const;
%rename(IndexInto) operator[](size_t);
%rename(Functor) operator ();
%rename(EqualEqual) operator ==;
%rename(NotEqual) operator !=;
%rename(LessThan) operator <;
%rename(LessThanEqual) operator <=;
%rename(GreaterThan) operator >;
%rename(GreaterThanEqual) operator >=;
%rename(And) operator &&;
%rename(Or) operator ||;
%rename(PlusPlusPrefix) operator++();
%rename(PlusPlusPostfix) operator++(int);
%rename(MinusMinusPrefix) operator--();
%rename(MinusMinusPostfix) operator--(int);
%rename(WriteIntoOp) operator <<;
%rename(WriteToOp) operator >>;
%rename(ToRadian) operator Radian() const;
%rename(ToDegree) operator Degree() const;

#endif



%typemap(cscode) cAudio::cVector3 %{

  public static bool operator>(cVector3 lhs, cVector3 rhs) 
  {
    return lhs.GreaterThan(rhs);
  }
  
  public static bool operator>=(cVector3 lhs, cVector3 rhs) 
  {
    return lhs.GreaterThan(rhs) || lhs.EqualEqual(rhs);
  }
  
  public static bool operator<(cVector3 lhs, cVector3 rhs) 
  {
    return lhs.LessThan(rhs);
  }
  
  public static bool operator<=(cVector3 lhs, cVector3 rhs) 
  {
    return lhs.LessThan(rhs) || lhs.EqualEqual(rhs);
  }
  
  public static cVector3 operator+(cVector3 lhs, cVector3 rhs) 
  {
    return lhs.Plus(rhs);
  }

  public static cVector3 operator*(cVector3 lhs, cVector3 rhs) 
  {
    return lhs.Multiply(rhs);
  }
  
  public static cVector3 operator*(cVector3 lhs, float rhs) 
  {
    return lhs.Multiply(rhs);
  }
  
  public static cVector3 operator/(cVector3 lhs, cVector3 rhs) 
  {
    return lhs.Divide(rhs);
  }
  
  public static cVector3 operator/(cVector3 lhs, float rhs) 
  {
    return lhs.Divide(rhs);
  }
     
  public static cVector3 operator-(cVector3 lhs, cVector3 rhs) 
  {
    return lhs.Minus(rhs);
  }
%}

%feature("director") cAudio::ILogReceiver;
%feature("director") cAudio::ISourceEventHandler;
%feature("director") cAudio::ICaptureEventHandler;
%feature("director") cAudio::IDataSource;
%feature("director") cAudio::IDataSourceFactory;
%feature("director") cAudio::IManagerEventHandler;
%feature("director") cAudio::IAudioDecoderFactory;
%feature("director") cAudio::IAudioDecoder;

%include "..\cAudio\include\cAudioString.h"
%include "..\cAudio\include\IMemoryProvider.h"
%include "..\cAudio\include\cAudioDefines.h"
%include "..\cAudio\include\cAudioMemory.h"
%include "..\cAudio\include\cAudioPlatform.h"
%include "..\cAudio\include\cAudioSleep.h"
%include "..\cAudio\include\cAudioString.h"
%include "..\cAudio\include\IAudioDeviceList.h"
%include "..\cAudio\include\EAudioFormats.h"
%include "..\cAudio\include\IRefCounted.h"
%include "..\cAudio\include\IDataSource.h"
%include "..\cAudio\include\IDataSourceFactory.h"
%include "..\cAudio\include\cVector3.h"
%include "..\cAudio\include\IListener.h"
%include "..\cAudio\include\ILogReceiver.h"
%include "..\cAudio\include\ILogger.h"
%include "..\cAudio\include\IManagerEventHandler.h"
%include "..\cAudio\include\ISourceEventHandler.h"
%include "..\cAudio\include\ICaptureEventHandler.h"
%include "..\cAudio\include\IEffectParameters.h"
%include "..\cAudio\include\IFilter.h"
%include "..\cAudio\include\IEffect.h"
%include "..\cAudio\include\IAudioEffects.h"
%include "..\cAudio\include\IAudioCapture.h"
%include "..\cAudio\include\IAudioDecoder.h"
%include "..\cAudio\include\IAudioDecoderFactory.h"
%include "..\cAudio\include\IAudioManager.h"
%include "..\cAudio\include\IAudioDeviceContext.h"
%include "..\cAudio\include\IAudioPlugin.h"
%include "..\cAudio\include\IAudioSource.h"
%include "..\cAudio\include\IPluginManager.h"
%include "..\cAudio\include\IThread.h"
%include "..\cAudio\include\cAudio.h"

namespace std {
   %template(IntVector) vector<int>;
   %template(DoubleVector) vector<double>;
   %template(StringVector) vector<std::string>;
   %template(StringMap) map<std::string, std::string>;
};

%pointer_functions(float, floatp);
