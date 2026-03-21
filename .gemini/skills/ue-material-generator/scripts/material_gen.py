import unreal
import json
import argparse
import os

class MaterialManager:
    def __init__(self):
        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        self.base_path = "/Game/Materials/Generated/"
        
        # Mapping of parameter names to Material Property inputs
        self.property_map = {
            "BaseColor": unreal.MaterialProperty.MP_BASE_COLOR,
            "Metallic": unreal.MaterialProperty.MP_METALLIC,
            "Specular": unreal.MaterialProperty.MP_SPECULAR,
            "Roughness": unreal.MaterialProperty.MP_ROUGHNESS,
            "Anisotropy": unreal.MaterialProperty.MP_ANISOTROPY,
            "EmissiveColor": unreal.MaterialProperty.MP_EMISSIVE_COLOR,
            "Opacity": unreal.MaterialProperty.MP_OPACITY,
            "OpacityMask": unreal.MaterialProperty.MP_OPACITY_MASK,
            "Normal": unreal.MaterialProperty.MP_NORMAL,
            "Tangent": unreal.MaterialProperty.MP_TANGENT,
            "SubsurfaceColor": unreal.MaterialProperty.MP_SUBSURFACE_COLOR,
            "AmbientOcclusion": unreal.MaterialProperty.MP_AMBIENT_OCCLUSION,
            "Refraction": unreal.MaterialProperty.MP_REFRACTION
        }
        
        if not unreal.EditorAssetLibrary.does_directory_exist(self.base_path):
            unreal.EditorAssetLibrary.make_directory(self.base_path)

    def get_unique_name(self, name):
        full_path = f"{self.base_path}{name}"
        if not unreal.EditorAssetLibrary.does_asset_exist(full_path):
            return name
        for i in range(1, 100):
            test_name = f"{name}_{i:02d}"
            if not unreal.EditorAssetLibrary.does_asset_exist(f"{self.base_path}{test_name}"):
                return test_name
        return name

    def create_base_material(self, name, params):
        """Creates a comprehensive PBR base material."""
        name = self.get_unique_name(name)
        factory = unreal.MaterialFactoryNew()
        material = self.asset_tools.create_asset(name, self.base_path, unreal.Material, factory)
        
        if not material:
            return None

        # Set blend mode for transparency if Opacity is present
        if "Opacity" in params:
            material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
            material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)

        for param_name, property_id in self.property_map.items():
            if param_name in params:
                value = params[param_name]
                
                # Check if it's a vector (list) or scalar
                if isinstance(value, list):
                    expr = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionVectorParameter)
                    expr.set_editor_property("parameter_name", param_name)
                    # Ensure 4 components for LinearColor
                    v = value + [1.0] * (4 - len(value))
                    expr.set_editor_property("default_value", unreal.LinearColor(v[0], v[1], v[2], v[3]))
                else:
                    expr = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionScalarParameter)
                    expr.set_editor_property("parameter_name", param_name)
                    expr.set_editor_property("default_value", float(value))
                
                unreal.MaterialEditingLibrary.connect_material_property(expr, "", property_id)

        unreal.MaterialEditingLibrary.recompile_material(material)
        unreal.EditorAssetLibrary.save_asset(material.get_path_name())
        return material.get_path_name()

    def create_instance(self, name, parent_path, overrides):
        """Creates a material instance with any PBR overrides."""
        name = self.get_unique_name(name)
        parent = unreal.EditorAssetLibrary.load_asset(parent_path)
        if not parent:
            unreal.log_error(f"Parent material not found: {parent_path}")
            return None

        factory = unreal.MaterialInstanceConstantFactoryNew()
        instance = self.asset_tools.create_asset(name, self.base_path, unreal.MaterialInstanceConstant, factory)
        unreal.MaterialEditingLibrary.set_material_instance_parent(instance, parent)

        for key, value in overrides.items():
            if isinstance(value, list):
                v = value + [1.0] * (4 - len(value))
                unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
                    instance, key, unreal.LinearColor(v[0], v[1], v[2], v[3]))
            else:
                unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(instance, key, float(value))

        unreal.EditorAssetLibrary.save_asset(instance.get_path_name())
        unreal.EditorAssetLibrary.sync_browser_to_objects([instance.get_path_name()])
        return instance.get_path_name()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", choices=["base", "instance"], required=True)
    parser.add_argument("--name", required=True)
    parser.add_argument("--params", type=str, default="{}")
    parser.add_argument("--parent", type=str, default="")
    
    args = parser.parse_args()
    try:
        params = json.loads(args.params)
    except:
        params = {}
    
    mgr = MaterialManager()
    if args.mode == "base":
        result = mgr.create_base_material(args.name, params)
    else:
        result = mgr.create_instance(args.name, args.parent, params)
    
    if result:
        print(f"SUCCESS:{result}")
