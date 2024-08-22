import os
import re
import matplotlib.pyplot as plt

def parse_map_file(map_file_path):
    memory_sections = {}
    current_section = None

    with open(map_file_path, 'r') as file:
        for line in file:
            # Match section headers
            section_match = re.match(r'^\s*(\.[a-zA-Z0-9_]+)\s+(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)', line)
            if section_match:
                section_name = section_match.group(1)
                section_address = int(section_match.group(2), 16)
                section_size = int(section_match.group(3), 16)
                memory_sections[section_name] = {
                    'address': section_address,
                    'size': section_size,
                    'symbols': []
                }
                current_section = section_name
            elif current_section:
                # Match symbols within sections
                symbol_match = re.match(r'^\s*0x[0-9a-fA-F]+\s+(\S+)', line)
                if symbol_match:
                    symbol_name = symbol_match.group(1)
                    memory_sections[current_section]['symbols'].append(symbol_name)
    
    return memory_sections

def print_memory_summary(memory_sections):
    print("Memory Usage Summary")
    print("====================\n")
    total_memory = 0
    
    for section, details in memory_sections.items():
        print(f"Section: {section}")
        print(f"  Address: 0x{details['address']:08X}")
        print(f"  Size: {details['size']} bytes")
        print(f"  Symbols: {len(details['symbols'])} symbols")
        if section == '.dtors':  # Add detailed symbol output for .dtors section
            print("  Detailed Symbols:")
            for symbol in details['symbols']:
                print(f"    {symbol}")
        print()
        total_memory += details['size']
    
    print(f"Total Memory Used: {total_memory} bytes")

def plot_memory_map(memory_sections):
    sections = list(memory_sections.keys())
    sizes = [details['size'] for details in memory_sections.values()]

    fig, ax = plt.subplots()
    ax.barh(sections, sizes, color='skyblue')
    ax.set_xlabel('Size (bytes)')
    ax.set_title('Memory Map Visualization')
    plt.show()

if __name__ == "__main__":
    map_file_path = "/home/smith/Agon/mystuff/agon-vdp-otf/.pio/build/esp32dev/firmware.map"
    
    if not os.path.exists(map_file_path):
        print(f"Error: The map file does not exist at the specified path: {map_file_path}")
    else:
        memory_sections = parse_map_file(map_file_path)
        print_memory_summary(memory_sections)
        plot_memory_map(memory_sections)
