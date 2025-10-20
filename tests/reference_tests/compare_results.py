import os
import re
from pathlib import Path
import numpy as np

def read_openfoam_field(file_path):
    """
    Reads an OpenFOAM field file (scalar or vector, uniform or nonuniform)
    and returns its internalField values as a NumPy array.
    """
    path = Path(file_path)
    if not path.exists():
        raise FileNotFoundError(f"{path} does not exist")

    content = path.read_text()

    # Extract internalField definition
    match = re.search(r'internalField\s+([^;]+);', content)
    if not match:
        print(f"No 'internalField' found in {path}")
        return None

    field_text = match.group(1).strip()

    # 🟩 Case 1: Uniform field
    if field_text.startswith("uniform"):
        val_str = field_text[len("uniform"):].strip()
        if val_str.startswith("("):
            # Vector or tensor
            values = np.fromstring(val_str.strip("()"), sep=" ")
        else:
            # Scalar
            values = np.array([float(val_str)])   # ← Force NumPy array
        return values

    # 🟦 Case 2: Nonuniform field
    elif "nonuniform" in field_text:
        type_match = re.search(r'nonuniform\s+List<(\w+)>', field_text)
        field_type = type_match.group(1).lower() if type_match else "scalar"

        block_match = re.search(
            r'nonuniform\s+List<\w+>\s+(\d+)\s*\((.*?)\)\s*;',
            content,
            re.DOTALL
        )
        if not block_match:
            raise ValueError("Cannot parse nonuniform field data")

        count = int(block_match.group(1))
        data_block = block_match.group(2).strip()

        if field_type == "scalar":
            values = np.fromstring(data_block, sep="\n")
        elif field_type == "vector":
            vecs = re.findall(r'\(([^)]+)\)', data_block)
            values = np.array([list(map(float, v.split())) for v in vecs])
        else:
            raise NotImplementedError(f"Unsupported field type: {field_type}")

        if len(values) != count:
            raise ValueError(f"Expected {count} entries, got {len(values)}")

        return values

    else:
        raise ValueError("Unrecognized internalField format")



def compare_openfoam_cases(case_folder, reference_folder, tolerance=1e-6):
    """
    Compare all files in OpenFOAM case folder against reference folder recursively.
    """
    errors = []
    
    for root, dirs, files in os.walk(case_folder):
        for file_name in files:
            case_file = os.path.join(root, file_name)
            rel_path = os.path.relpath(case_file, case_folder)
            ref_file = os.path.join(reference_folder, rel_path)
            
            if not os.path.exists(ref_file):
                errors.append(f"Reference file missing: {ref_file}")
                continue

            case_data = read_openfoam_field(case_file)
            ref_data = read_openfoam_field(ref_file)

            # Skip non-numeric files
            if case_data is None or ref_data is None:
                print(f"Skipping non-numeric file: {rel_path}")
                continue
            
            if case_data.shape != ref_data.shape:
                errors.append(f"Shape mismatch for {rel_path}: {case_data.shape} vs {ref_data.shape}")
                continue
            
            l2_diff = np.linalg.norm(case_data - ref_data)
            if l2_diff > tolerance:
                errors.append(f"L2 norm too large for {rel_path}: {l2_diff}")
    
    if errors:
        error_message = "Differences found:\n" + "\n".join(f"  {e}" for e in errors)
        print(error_message)
        raise ValueError(error_message)
    else:
        print("All numeric fields match within tolerance.")

if __name__ == "__main__":
    import argparse
    import sys

    parser = argparse.ArgumentParser(description="Compare OpenFOAM result folders recursively.")
    parser.add_argument("--case_folder", help="Folder containing simulation results to check")
    parser.add_argument("--reference_folder", help="Folder containing reference results")
    parser.add_argument("--tolerance", type=float, default=1e-6, help="L2 norm tolerance")
    
    args = parser.parse_args()
    
    try:
        compare_openfoam_cases(args.case_folder, args.reference_folder, args.tolerance)
    except ValueError as e:
        print(f"Error: {e}")
        sys.exit(1)
