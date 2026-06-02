#include <tinyxml2.h>
#include <tinyxml.h>

#include <urdf/model.h>
#include <urdf_parser/urdf_parser.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
// #include <string>
void fixURDF(TiXmlElement* element)
{
    if (!element) return;

    // Fix color values
    if (std::string(element->Value()) == "color")
    {
        const char* rgba = element->Attribute("rgba");

        if (rgba)
        {
            std::stringstream ss(rgba);

            double r, g, b, a;
            ss >> r >> g >> b >> a;

            r = std::clamp(r, 0.0, 1.0);
            g = std::clamp(g, 0.0, 1.0);
            b = std::clamp(b, 0.0, 1.0);
            a = std::clamp(a, 0.0, 1.0);

            std::ostringstream out;
            out << r << " " << g << " "
                << b << " " << a;

            element->SetAttribute("rgba", out.str());
        }
    }

    // Fix limits
    if (std::string(element->Value()) == "limit")
    {
        const char* effort = element->Attribute("effort");
        const char* velocity = element->Attribute("velocity");

        if (effort && std::string(effort) == "inf")
            element->SetAttribute("effort", "1000");

        if (velocity && std::string(velocity) == "inf")
            element->SetAttribute("velocity", "100");

        const char* lower = element->Attribute("lower");
        const char* upper = element->Attribute("upper");

        if (lower)
            element->SetAttribute("lower", "-1000");

        if (upper)
            element->SetAttribute("upper", "1000");
    }

    // Fix joints
    if (std::string(element->Value()) == "joint")
    {
        const char* name = element->Attribute("name");
        const char* type = element->Attribute("type");

        if (type && std::string(type) == "fixed")
        {
            TiXmlElement* axis = element->FirstChildElement("axis");

            if (axis)
            {
                element->RemoveChild(axis);
            }
        }

        if (type)
        {
            std::string joint_type(type);

            if (joint_type == "floating")
            {
                element->SetAttribute("type", "fixed");
            }

            if (name)
            {
                std::string joint_name(name);

                if (joint_name.find("wheel") != std::string::npos)
                {
                    element->SetAttribute("type", "continuous");
                }
            }
        }
    }

    // Recurse through children
    for (TiXmlElement* child = element->FirstChildElement();
         child;
         child = child->NextSiblingElement())
    {
        fixURDF(child);
    }
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: sdf_to_urdf <input_sdf_file> <output_urdf_file>" << std::endl;
        return 1;
    }

    const std::string sdf_file = argv[1];
    const std::string output_file = argv[2];

    // Read SDF file
    std::ifstream file(sdf_file);
    if (!file)
    {
        std::cerr << "Failed to open SDF file: " << sdf_file << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sdf_str = buffer.str();

    // Parse SDF with TinyXML2
    tinyxml2::XMLDocument sdf_doc;

    if (sdf_doc.Parse(sdf_str.c_str()) != tinyxml2::XML_SUCCESS)
    {
        std::cerr << "Failed to parse XML in SDF file." << std::endl;
        return 1;
    }

    // Find <sdf>
    tinyxml2::XMLElement *sdf_elem =
        sdf_doc.FirstChildElement("sdf");

    if (!sdf_elem)
    {
        std::cerr << "No <sdf> tag found." << std::endl;
        return 1;
    }

    // Find <model>
    tinyxml2::XMLElement *model_elem =
        sdf_elem->FirstChildElement("model");

    if (!model_elem)
    {
        std::cerr << "No <model> tag found." << std::endl;
        return 1;
    }

    // Remove top-level model pose if present
    tinyxml2::XMLElement *pose_elem =
        model_elem->FirstChildElement("pose");

    if (pose_elem)
    {
        std::cout << "Found model pose. Removing it before conversion..."
                  << std::endl;

        model_elem->DeleteChild(pose_elem);
    }

    // Convert XML back to string
    tinyxml2::XMLPrinter printer;
    sdf_doc.Print(&printer);

    std::string modified_sdf = printer.CStr();

    // Parse with sdformat_urdf through urdf::Model
    urdf::Model model;

    if (!model.initString(modified_sdf))
    {
        std::cerr << "Failed to parse SDF into URDF model." << std::endl;
        return 1;
    }

    std::cout << "Successfully loaded model: "
              << model.getName()
              << std::endl;

    // Export URDF
    TiXmlDocument *urdf_xml_doc =
        urdf::exportURDF(model);

    TiXmlElement* root = urdf_xml_doc->RootElement();
    fixURDF(root);

    if (!urdf_xml_doc)
    {
        std::cerr << "Failed to export URDF." << std::endl;
        return 1;
    }

    // Save URDF
    bool success =
        urdf_xml_doc->SaveFile(output_file.c_str());

    if (!success)
    {
        std::cerr << "Failed to save URDF to "
                  << output_file
                  << std::endl;

        delete urdf_xml_doc;
        return 1;
    }

    std::cout << "URDF saved to: "
              << output_file
              << std::endl;

    delete urdf_xml_doc;

    return 0;
}