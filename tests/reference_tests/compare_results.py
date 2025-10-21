import os
import numpy as np

import re
import numpy as np

import numpy as np
import re

def read_openfoam_field(file_path):
    """
    Reads an OpenFOAM field (scalar or vector) from file and returns a NumPy array.
    Handles uniform and nonuniform fields.
    """
    with open(file_path, 'r') as f:
        lines = [line.strip() for line in f if line.strip() and not line.strip().startswith('//')]

    if not lines:
        return None

    # Search for internalField line
    for i, line in enumerate(lines):
        if line.startswith('internalField'):
            internal_idx = i
            break
    else:
        return None

    line = lines[internal_idx]

    # Uniform field: "internalField uniform VALUE;"
    m = re.match(r"internalField\s+uniform\s+(.+);", line)
    if m:
        val_str = m.group(1).strip()
        if val_str.startswith('('):  # vector
            value = np.array([float(x) for x in val_str[1:-1].split()])
        else:
            value = float(val_str)
        return np.array([value])

    # Nonuniform field: "internalField nonuniform List<type>"
    if 'nonuniform' in line:
        # Next line may be number of entries
        num_entries = int(lines[internal_idx + 1])
        # Find starting '('
        start_idx = internal_idx + 2
        while start_idx < len(lines) and '(' not in lines[start_idx]:
            start_idx += 1
        start_idx += 1  # line after '('
        # Find ending ')'
        end_idx = start_idx
        while end_idx < len(lines) and ')' not in lines[end_idx]:
            end_idx += 1

        data_lines = lines[start_idx:end_idx]
        data = []
        for l in data_lines:
            l = l.replace('(', '').replace(')', '').replace(';', '').strip()
            if not l:
                continue
            vals = [float(x) for x in l.split()]
            data.append(vals if len(vals) > 1 else vals[0])
        return np.array(data)

    # Fallback: attempt to parse numbers directly from the line
    numbers = re.findall(r"[-+]?\d*\.\d+|\d+", line)
    if numbers:
        return np.array([float(x) for x in numbers])

    return None


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
        print("Differences found:")
        for e in errors:
            print("  ", e)
    else:
        print("All numeric fields match within tolerance.")

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Compare OpenFOAM result folders recursively.")
    parser.add_argument("case_folder", help="Folder containing simulation results to check")
    parser.add_argument("reference_folder", help="Folder containing reference results")
    parser.add_argument("--tolerance", type=float, default=1e-6, help="L2 norm tolerance")
    
    args = parser.parse_args()
    
    compare_openfoam_cases(args.case_folder, args.reference_folder, args.tolerance)
