"""Small local client for CodexUnrealBridge. Run while Unreal Editor is open."""

import argparse
import json
import pathlib
import sys
import time

ENGINE_ROOT = pathlib.Path(r"H:\UE_5.7")
sys.path.insert(0, str(ENGINE_ROOT / "Engine/Plugins/Experimental/PythonScriptPlugin/Content/Python"))

import remote_execution


def run(command: str, mode: str = remote_execution.MODE_EVAL_STATEMENT):
    session = remote_execution.RemoteExecution()
    session.start()
    try:
        deadline = time.time() + 6.0
        while time.time() < deadline and not session.remote_nodes:
            time.sleep(0.1)
        if not session.remote_nodes:
            raise RuntimeError("没有找到 Unreal Editor。请先打开项目，并确认 Python Remote Execution 已启用。")
        if len(session.remote_nodes) > 1:
            names = [node.get("project_name", node.get("node_id", "unknown")) for node in session.remote_nodes]
            raise RuntimeError("发现多个 Unreal Editor，请只保留目标项目：" + ", ".join(names))
        session.open_command_connection(session.remote_nodes[0]["node_id"])
        return session.run_command(command, exec_mode=mode, raise_on_failure=True)
    finally:
        session.stop()


def bridge_expression(method: str, *args):
    encoded = ", ".join(repr(value) for value in args)
    return f"unreal.get_editor_subsystem(unreal.CodexUnrealBridgeSubsystem).{method}({encoded})"


def main():
    parser = argparse.ArgumentParser(description="Connect to the open ChickenPlaguePolice Unreal Editor")
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("status")
    sub.add_parser("scene")
    bp = sub.add_parser("blueprint")
    bp.add_argument("asset_path")
    raw = sub.add_parser("eval")
    raw.add_argument("expression")
    args = parser.parse_args()

    if args.command == "status":
        expression = bridge_expression("get_bridge_version")
    elif args.command == "scene":
        expression = bridge_expression("get_current_level_snapshot") + ".json"
    elif args.command == "blueprint":
        expression = bridge_expression("get_blueprint_snapshot", args.asset_path) + ".json"
    else:
        expression = args.expression

    result = run(expression)
    print(json.dumps(result, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
