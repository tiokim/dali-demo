/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "image-channel-control-impl.h"
#include <dali-toolkit/dali-toolkit.h>
#include <dali/public-api/object/type-registry-helper.h>
#include <dali-toolkit/devel-api/align-enums.h>
#include <dali-toolkit/devel-api/visual-factory/visual-factory.h>
#include <dali-toolkit/devel-api/visual-factory/devel-visual-properties.h>

#include <cstdio>

using namespace Dali; // Needed for macros

namespace Demo
{
namespace Internal
{

namespace
{

const char* FRAGMENT_SHADER = DALI_COMPOSE_SHADER(
  varying mediump vec2 vTexCoord;\n
  uniform sampler2D sTexture;\n
  uniform mediump vec4 uColor;\n
  uniform mediump vec3 uChannels;\n
  \n
  void main()\n
  {\n
    gl_FragColor = texture2D( sTexture, vTexCoord ) * uColor * vec4(uChannels, 1.0) ;\n
  }\n
);

Dali::BaseHandle Create()
{
  return Demo::ImageChannelControl::New();
}

DALI_TYPE_REGISTRATION_BEGIN( ImageChannelControl, Dali::Toolkit::Control, Create );

DALI_PROPERTY_REGISTRATION( Demo, ImageChannelControl, "url", STRING, RESOURCE_URL );
DALI_PROPERTY_REGISTRATION( Demo, ImageChannelControl, "redChannel", FLOAT, RED_CHANNEL );
DALI_PROPERTY_REGISTRATION( Demo, ImageChannelControl, "greenChannel", FLOAT, GREEN_CHANNEL );
DALI_PROPERTY_REGISTRATION( Demo, ImageChannelControl, "blueChannel", FLOAT, BLUE_CHANNEL );

DALI_PROPERTY_REGISTRATION( Demo, ImageChannelControl, "visibility", BOOLEAN, VISIBILITY );
DALI_PROPERTY_REGISTRATION( Demo, ImageChannelControl, "enableVisibilityTransition", ARRAY, ENABLE_VISIBILITY_TRANSITION );
DALI_PROPERTY_REGISTRATION( Demo, ImageChannelControl, "disableVisibilityTransition", ARRAY, DISABLE_VISIBILITY_TRANSITION );

DALI_PROPERTY_REGISTRATION( Demo, ImageChannelControl, "imageVisual", MAP, IMAGE_VISUAL );
DALI_TYPE_REGISTRATION_END();

} // anonymous namespace


Internal::ImageChannelControl::ImageChannelControl()
: Control( ControlBehaviour( REQUIRES_STYLE_CHANGE_SIGNALS ) ),
  mChannels( 1.0f, 1.0f, 1.0f ),
  mChannelIndex( Property::INVALID_INDEX ),
  mVisibility(true),
  mTargetVisibility(true)
{
}

Internal::ImageChannelControl::~ImageChannelControl()
{
}

Demo::ImageChannelControl Internal::ImageChannelControl::New()
{
  IntrusivePtr<Internal::ImageChannelControl> impl = new Internal::ImageChannelControl();
  Demo::ImageChannelControl handle = Demo::ImageChannelControl( *impl );
  impl->Initialize();
  return handle;
}

void ImageChannelControl::SetImage( const std::string& url )
{
  mUrl = url;

  Actor self = Self();

  Property::Map properties;
  Property::Map shader;
  shader[Dali::Toolkit::Visual::Shader::Property::FRAGMENT_SHADER] = FRAGMENT_SHADER;
  properties[Dali::Toolkit::Visual::Property::TYPE] = Dali::Toolkit::Visual::IMAGE;
  properties[Dali::Toolkit::Visual::Property::SHADER]=shader;
  properties[Dali::Toolkit::ImageVisual::Property::URL] = url;

  mVisual = Toolkit::VisualFactory::Get().CreateVisual( properties );
  RegisterVisual( Demo::ImageChannelControl::Property::IMAGE_VISUAL, mVisual );
  mVisual.SetName("imageVisual");

  RelayoutRequest();
}

void ImageChannelControl::SetVisibility( bool visibility )
{
  printf("ImageChannelControl %s: SetVisibility( %s )\n", Self().GetName().c_str(), visibility?"T":"F" );

  Animation animation;

  if( mAnimation )
  {
    mAnimation.Stop();
    mAnimation.FinishedSignal().Disconnect( this, &ImageChannelControl::OnStateChangeAnimationFinished );
    OnStateChangeAnimationFinished(mAnimation);
  }

  if( mVisibility != visibility )
  {
    if( mVisibility )
    {
      if( mDisableVisibilityTransition.Count() > 0 )
      {
        mAnimation = CreateTransition( mDisableVisibilityTransition );
      }
    }
    else
    {
      if( mEnableVisibilityTransition.Count() > 0 )
      {
        mAnimation = CreateTransition( mEnableVisibilityTransition );
      }
    }
  }

  if( mAnimation )
  {
    mAnimation.FinishedSignal().Connect( this, &ImageChannelControl::OnStateChangeAnimationFinished );
    mAnimation.Play();
    mTargetVisibility = visibility;
  }
  else
  {
    mVisibility = visibility;
  }
}

void ImageChannelControl::OnStateChangeAnimationFinished( Animation& src )
{
  mVisibility = mTargetVisibility;
}

void ImageChannelControl::OnInitialize()
{
  Actor self = Self();
  mChannelIndex = self.RegisterProperty( "uChannels", Vector3(1.0f, 1.0f, 1.0f) );
}

void ImageChannelControl::OnStageConnection( int depth )
{
  Control::OnStageConnection( depth );
}

void ImageChannelControl::OnStageDisconnection()
{
  Control::OnStageDisconnection();
}

void ImageChannelControl::OnSizeSet( const Vector3& targetSize )
{
  Control::OnSizeSet( targetSize );

  if( mVisual )
  {
    Vector2 size( targetSize );
    Property::Map transformMap;
    transformMap
      .Add( Toolkit::Visual::DevelProperty::Transform::Property::OFFSET, Vector2(0.0f, 0.0f) )
      .Add( Toolkit::Visual::DevelProperty::Transform::Property::SIZE, Vector2(1.0f, 1.0f) )
      .Add( Toolkit::Visual::DevelProperty::Transform::Property::ORIGIN, Toolkit::Align::CENTER )
      .Add( Toolkit::Visual::DevelProperty::Transform::Property::ANCHOR_POINT, Toolkit::Align::CENTER )
      .Add( Toolkit::Visual::DevelProperty::Transform::Property::OFFSET_SIZE_MODE, Vector4::ZERO );

    mVisual.SetTransformAndSize( transformMap, size );
  }
}

Vector3 ImageChannelControl::GetNaturalSize()
{
  if( mVisual )
  {
    Vector2 naturalSize;
    mVisual.GetNaturalSize(naturalSize);
    return Vector3(naturalSize);
  }
  return Vector3::ZERO;
}

void ImageChannelControl::OnStyleChange( Toolkit::StyleManager styleManager, StyleChange::Type change )
{
  // Chain up.
  Control::OnStyleChange( styleManager, change );
}


///////////////////////////////////////////////////////////
//
// Properties
//

void ImageChannelControl::SetProperty( BaseObject* object, Property::Index index, const Property::Value& value )
{
  Demo::ImageChannelControl imageChannelControl = Demo::ImageChannelControl::DownCast( Dali::BaseHandle( object ) );

  if( imageChannelControl )
  {
    ImageChannelControl& impl = GetImpl( imageChannelControl );
    Actor self = impl.Self();
    switch ( index )
    {
      case Demo::ImageChannelControl::Property::RESOURCE_URL:
      {
        impl.SetImage( value.Get<std::string>() );
        break;
      }
      case Demo::ImageChannelControl::Property::IMAGE_VISUAL:
      {
        Property::Map* map = value.GetMap();
        if( map )
        {
          impl.mVisual = Toolkit::VisualFactory::Get().CreateVisual( *map );
          impl.RegisterVisual( Demo::ImageChannelControl::Property::IMAGE_VISUAL, impl.mVisual );
        }
        break;
      }
      case Demo::ImageChannelControl::Property::VISIBILITY:
      {
        impl.SetVisibility( value.Get<bool>() );
        break;
      }
      case Demo::ImageChannelControl::Property::ENABLE_VISIBILITY_TRANSITION:
      {
        if( value.GetType() == Property::ARRAY )
        {
          impl.mEnableVisibilityTransition = Toolkit::TransitionData::New( *value.GetArray());
        }
        else if( value.GetType() == Property::MAP )
        {
          impl.mEnableVisibilityTransition = Toolkit::TransitionData::New( *value.GetMap() );
        }
        break;
      }
      case Demo::ImageChannelControl::Property::DISABLE_VISIBILITY_TRANSITION:
      {
        if( value.GetType() == Property::ARRAY )
        {
          impl.mDisableVisibilityTransition = Toolkit::TransitionData::New( *value.GetArray());
        }
        else if( value.GetType() == Property::MAP )
        {
          impl.mDisableVisibilityTransition = Toolkit::TransitionData::New( *value.GetMap() );
        }
        break;
      }
      case Demo::ImageChannelControl::Property::RED_CHANNEL:
      {
        impl.mChannels[0] = value.Get<float>();
        self.SetProperty( impl.mChannelIndex, impl.mChannels );
        break;
      }
      case Demo::ImageChannelControl::Property::GREEN_CHANNEL:
      {
        impl.mChannels[1] = value.Get<float>();
        self.SetProperty( impl.mChannelIndex, impl.mChannels );
        break;
      }
      case Demo::ImageChannelControl::Property::BLUE_CHANNEL:
      {
        impl.mChannels[2] = value.Get<float>();
        self.SetProperty( impl.mChannelIndex, impl.mChannels );
        break;
      }
    }
  }
}

Property::Value ImageChannelControl::GetProperty( BaseObject* object, Property::Index propertyIndex )
{
  Property::Value value;

  Demo::ImageChannelControl imageChannelControl = Demo::ImageChannelControl::DownCast( Dali::BaseHandle( object ) );

  if ( imageChannelControl )
  {
    ImageChannelControl& impl = GetImpl( imageChannelControl );
    switch ( propertyIndex )
    {
      case Demo::ImageChannelControl::Property::RED_CHANNEL:
      {
        value = impl.mChannels[0];
        break;
      }
      case Demo::ImageChannelControl::Property::GREEN_CHANNEL:
      {
        value = impl.mChannels[1];
        break;
      }
      case Demo::ImageChannelControl::Property::BLUE_CHANNEL:
      {
        value = impl.mChannels[2];
        break;
      }
      case Demo::ImageChannelControl::Property::VISIBILITY:
      {
        value = impl.mVisibility;
        break;
      }
      default:
        break;
    }
  }

  return value;
}

} // Internal
} // Demo
