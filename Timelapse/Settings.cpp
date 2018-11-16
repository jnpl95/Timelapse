#include "Settings.h"
#include "Log.h"

using namespace Timelapse;
using namespace System;
using namespace IO;
using namespace Collections::Generic;

Object^ Settings::Deserialize(String^ path, XmlSerializer^ serializer)
{
	if (File::Exists(path))
	{
		FileStream^ stream = {};

		try
		{
			stream = gcnew FileStream(path, FileMode::Open, FileAccess::Read, FileShare::Read);
			return serializer->Deserialize(stream);
		}

		catch (Exception^ ex)
		{
			Log::WriteLine("Exception occured while reading from " + path + " : " + ex->Message);
		}

		finally
		{
			if (stream) 
				delete static_cast<IDisposable^>(stream);
				Log::WriteLine("Loaded " + path);
		}
	}

	return nullptr;
}

Void Settings::Serialize(String^ path, XmlSerializer^ serializer, Object^ object)
{
	if (path != nullptr && serializer != nullptr && object != nullptr)
	{
		FileStream^ stream = {};

		try
		{
			stream = gcnew FileStream(path, FileMode::Create, FileAccess::Write, FileShare::None);
			serializer->Serialize(stream, object);
		}

		catch (Exception^ ex)
		{
			Log::WriteLine("Exception occured while writing to " + path + " : " + ex->Message);
		}

		finally
		{
			if (stream) 
				delete static_cast<IDisposable^>(stream);
				Log::WriteLine("Saved " + path);
		}
	}
}

Void Settings::Serialize(Control^ c, String^ XmlFileName)
{
	XmlTextWriter^ xmlSerializedForm = gcnew XmlTextWriter(XmlFileName, System::Text::Encoding::Default);

	xmlSerializedForm->Formatting = Formatting::Indented;
	xmlSerializedForm->WriteStartDocument();
	xmlSerializedForm->WriteStartElement("Timelapse");

	// enumerate all controls on the form, and serialize them as appropriate
	AddChildControls(xmlSerializedForm, c);

	xmlSerializedForm->WriteEndElement();
	xmlSerializedForm->WriteEndDocument();
	xmlSerializedForm->Flush();
	xmlSerializedForm->Close();

	Log::WriteLine("Saved " + XmlFileName);
}

Void Settings::AddChildControls(XmlTextWriter^ xmlSerializedForm, Control^ c)
{
	for each(Control^ childCtrl in c->Controls)
	{
		auto type = childCtrl->GetType();
		auto name = childCtrl->Name;

		//TODO: save press state of buttons?

		if (childCtrl->HasChildren || type == ComboBox::typeid || type == NumericUpDown::typeid || type == CheckBox::typeid || type == TextBox::typeid) // || name == "lbItemFilter"
		{
			// serialize this control
			xmlSerializedForm->WriteStartElement("Control");
			xmlSerializedForm->WriteAttributeString("Name", childCtrl->Name);

			if (type == ComboBox::typeid)
				xmlSerializedForm->WriteAttributeString("SelectedIndex", safe_cast<ComboBox^>(childCtrl)->SelectedIndex.ToString());
			else if (type == NumericUpDown::typeid)
				xmlSerializedForm->WriteAttributeString("Value", safe_cast<NumericUpDown^>(childCtrl)->Value.ToString());
			else if (type == CheckBox::typeid)
				xmlSerializedForm->WriteAttributeString("Checked", safe_cast<CheckBox^>(childCtrl)->Checked.ToString());
			else if (type == TextBox::typeid)
				xmlSerializedForm->WriteAttributeString("Text", safe_cast<TextBox^>(childCtrl)->Text->ToString());

			//TODO: save itemFilter as array of strings
			//else if (name == "lbItemFilter")				
			//xmlSerializedForm->WriteAttributeString("ItemFilterList", safe_cast<ListBox^>(childCtrl)->Items->ToString());

			// see if this control has any children, and if so, serialize them
			if (childCtrl->HasChildren && type != NumericUpDown::typeid) 
				AddChildControls(xmlSerializedForm, childCtrl);

			xmlSerializedForm->WriteEndElement(); // Control
		}
	}
}

Void Settings::Deserialize(Control^ c, String^ XmlFileName)
{
	if (File::Exists(XmlFileName))
	{
		try
		{
			XmlDocument^ xmlSerializedForm = gcnew XmlDocument();
			xmlSerializedForm->Load(XmlFileName);

			XmlNode^ topLevel = xmlSerializedForm->ChildNodes[1];
			for each(XmlNode^ n in topLevel->ChildNodes) 
				SetControlProperties(safe_cast<Control^>(c), n);
		}

		catch (Exception^ ex)
		{
			Log::WriteLine("While deserializing \"" + c->Name + "\" the following exception occured: \"" + ex->Message + "\"");
		}
	}
}

Void Settings::SetControlProperties(Control^ currentCtrl, XmlNode^ n)
{
	try
	{
		// get the control's name and type
		String^ controlName = n->Attributes["Name"]->Value;

		// find the control
		auto ctrl = currentCtrl->Controls->Find(controlName, true);

		// the right type too
		if (n->Attributes["SelectedIndex"])
			safe_cast<ComboBox^>(ctrl[0])->SelectedIndex = Convert::ToInt32(n->Attributes["SelectedIndex"]->Value);
		else if (n->Attributes["Value"])
			safe_cast<NumericUpDown^>(ctrl[0])->Value = Convert::ToDecimal(n->Attributes["Value"]->Value);
		else if (n->Attributes["Checked"])
			safe_cast<CheckBox^>(ctrl[0])->Checked = Convert::ToBoolean(n->Attributes["Checked"]->Value);
		else if (n->Attributes["Text"])
			safe_cast<TextBox^>(ctrl[0])->Text = Convert::ToString(n->Attributes["Text"]->Value);

		//TODO: parse array of strings and add by one to ListBox
		//else if (n->Attributes["ItemFilterList"])		
	    //safe_cast<ListBox^>(ctrl[0])->Items->Add(Convert::ToString(n->Attributes["ItemFilterList"]->Value));

		// if n has any children that are controls, deserialize them as well
		if (n->HasChildNodes && ctrl[0]->HasChildren)
		{
			XmlNodeList^ xnlControls = n->SelectNodes("Control");
			for each(XmlNode^ n2 in xnlControls) 
				SetControlProperties(ctrl[0], n2);
		}
	}

	catch (Exception^ ex)
	{
		Log::WriteLine("While deserializing \"" + n->Attributes["Name"]->Value + "\" the following exception occured: \"" + ex->Message + "\"");
	}
}

String^ Settings::GetSettingsPath() {

	String^ AppDataFolder = Environment::GetFolderPath(Environment::SpecialFolder::ApplicationData);
	String^ SettingsFilePath = {};

	try
	{
		String^ TimelapseFolderPath = Path::Combine(AppDataFolder, "Timelapse");
		SettingsFilePath = Path::Combine(TimelapseFolderPath, "Settings.xml");
	}

	catch (Exception^ ex)
	{
		Log::WriteLine("Exception occured while generating path to settings file" + ex->Message);
	}

	Log::WriteLine("Generated path to Timelapse settings file at" + ": " + SettingsFilePath);

	return SettingsFilePath;
}