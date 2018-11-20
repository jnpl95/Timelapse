#pragma once
using namespace System;
using namespace Windows::Forms;
using namespace Xml;
using namespace Serialization;
using namespace Collections::Generic;

namespace Timelapse {
	public ref class Settings {
		public:
			static Object^ Deserialize(String^ Path, XmlSerializer^ Deserializer);
			static void Deserialize(Control^ c, String^ Path);
			static void Serialize(String^ Path, XmlSerializer^ Serializer, Object^ Collection);
			static void Serialize(Control^ c, String^ Path);
			static String^ GetSettingsPath();

		private:
			static void AddChildControls(XmlTextWriter^ xmlSerialisedForm, Control^ c);
			static void SetControlProperties(Control^ currentCtrl, XmlNode^ n);
	};
}