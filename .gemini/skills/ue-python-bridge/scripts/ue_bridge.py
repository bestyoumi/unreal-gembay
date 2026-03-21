import sys
import os
import argparse
import time

# 1. Dynamically add the Unreal Python paths to sys.path to find remote_execution.py
ENGINE_PYTHON_PATH = r"C:\Program Files\Epic Games\UE_5.7\Engine\Plugins\Experimental\PythonScriptPlugin\Content\Python"
if os.path.exists(ENGINE_PYTHON_PATH):
    sys.path.append(ENGINE_PYTHON_PATH)
else:
    print(f"ERROR: Could not find Unreal Python path at {ENGINE_PYTHON_PATH}")
    sys.exit(1)

try:
    import remote_execution
except ImportError:
    print("ERROR: Failed to import remote_execution module from Unreal Engine.")
    sys.exit(1)

def run_command(command):
    """Executes a python command in the running Unreal Editor process."""
    remote_exec = remote_execution.RemoteExecution()
    remote_exec.start()
    
    # Wait for discovery
    time.sleep(1)
    nodes = remote_exec.remote_nodes
    
    target_node = None
    for node in nodes:
        # Debug print
        print(f"DEBUG: Node found: {node['node_id']} type: {node['command_line_type']}")
        if node.get('command_line_type') == 'Editor':
            target_node = node.get('node_id')
            break
    
    if not target_node:
        print("ERROR: No running Unreal Editor instance found. Ensure 'Enable Remote Execution' is on in Python Script Plugin settings.")
        remote_exec.stop()
        return False

    # Execute the command
    result = remote_exec.run_command(command, exec_mode='ExecuteStatement', target_node=target_node)
    
    remote_exec.stop()

    if result.get('success'):
        # Print output from the editor console
        output = result.get('output')
        if output:
            for line in output:
                print(f"[UE] {line['output']}")
        return True
    else:
        print(f"ERROR: Command failed in Unreal Editor.")
        return False

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Unreal Engine Python Bridge")
    parser.add_argument("command", help="The Python command or script to execute in UE")
    args = parser.parse_args()

    if run_command(args.command):
        sys.exit(0)
    else:
        sys.exit(1)
