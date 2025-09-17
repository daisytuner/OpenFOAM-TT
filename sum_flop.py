import json

with open("daisy_trace.json") as f:
    data = json.load(f)

    flops_by_function = {}
    for event in data["traceEvents"]:
        function = event["args"]["function"]
        if function not in flops_by_function:
            flops_by_function[function] = {"total": 0}

        metrics = event["args"]["metrics"]
        for metric in metrics:
            if metric not in flops_by_function[function]:
                flops_by_function[function][metric] = 0
            flops_by_function[function][metric] += metrics[metric]

            width = metric.split(":")[-1]
            if width == "SCALAR_UOPS_RETIRED":
                flops = metrics[metric] * 1
            elif width == "PACK128_UOPS_RETIRED":
                flops = metrics[metric] * 8
            elif width == "PACK256_UOPS_RETIRED":
                flops = metrics[metric] * 16
            elif width == "PACK512_UOPS_RETIRED":
                flops = metrics[metric] * 32
            else:
                flops = 0

            flops_by_function[function]["total"] += flops
    
    print(flops_by_function)

    sorted_flops = sorted(flops_by_function.items(), key=lambda x: x[1]["total"], reverse=True)
    # Print table header: Function, Value, FLOP
    print(f"| {'Function':<50} | {'FLOP':>25} | {'Metric':>50} | {'Count':>25} |")
    print(f"| {'-'*50} | {'-'*25} | {'-'*50} | {'-'*25} |")
    for function, metrics in sorted_flops:
        # Print function name, total flop, and all metrics
        # Format values with decimal commas
        print(f"| {function:<50} | {metrics['total']:>25,} | {'':<50} | {'':<25} |")
        for metric, value in metrics.items():
            if metric != "total":
                print(f"| {'':<50} | {'':>25} | {metric:<50} | {value:>25,} |")
