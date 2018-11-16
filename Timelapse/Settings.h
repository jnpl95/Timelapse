#pragma once
using namespace System;
using namespace Windows::Forms;
using namespace Xml;
using namespace Serialization;
using namespace Collections::Generic;

namespace Timelapse
{
	public ref class Settings
	{
	public:
		static Object^ Deserialize(String^ Path, XmlSerializer^ Deserializer);
		static Void Deserialize(Control^ c, String^ Path);
		static Void Serialize(String^ Path, XmlSerializer^ Serializer, Object^ Collection);
		static Void Serialize(Control^ c, String^ Path);
		static String^ GetSettingsPath();

	private:
		static Void AddChildControls(XmlTextWriter^ xmlSerialisedForm, Control^ c);
		static Void SetControlProperties(Control^ currentCtrl, XmlNode^ n);
	};
}