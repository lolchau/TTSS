# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import numpy as np
plt.rcParams['font.family'] = 'Arial'

methods = ['Tuan tu', 'Song song 1D', 'Song song 2D', 'Ring Topology']
times = [0.0079999, 0.00300002, 0.00300002, 0.0209999]
speedups = [1, 2.66661, 2.66661, 0.380949]

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))

colors = ['#FF6B6B', '#4ECDC4', '#45B7D1', '#FFA07A', '#98D8C8']
x_pos = np.arange(len(methods))

bars1 = ax1.bar(x_pos, times, color=colors[:len(methods)])
ax1.set_xlabel('Phuong phap', fontsize=12)
ax1.set_ylabel('Thoi gian (giay)', fontsize=12)
ax1.set_title('So sanh thoi gian thuc thi - Tich vo huong vector', fontsize=14, fontweight='bold')
ax1.set_xticks(x_pos)
ax1.set_xticklabels(methods, rotation=15, ha='right', fontsize=9)
ax1.grid(True, alpha=0.3, axis='y')
for bar in bars1:
    height = bar.get_height()
    ax1.text(bar.get_x() + bar.get_width()/2., height,
            f'{height:.6f}', ha='center', va='bottom', fontsize=8)

speedup_methods = methods[1:]
speedup_values = speedups[1:]
x_pos2 = np.arange(len(speedup_methods))
bars2 = ax2.bar(x_pos2, speedup_values, color=colors[1:len(methods)])
ax2.axhline(y=1, color='r', linestyle='--', label='Baseline', alpha=0.5)
ax2.set_xlabel('Phuong phap song song', fontsize=12)
ax2.set_ylabel('Toc do (Speedup)', fontsize=12)
ax2.set_title('Toc do tang toc so voi tuan tu', fontsize=14, fontweight='bold')
ax2.set_xticks(x_pos2)
ax2.set_xticklabels(speedup_methods, rotation=15, ha='right', fontsize=9)
ax2.legend(fontsize=10)
ax2.grid(True, alpha=0.3, axis='y')
for bar in bars2:
    height = bar.get_height()
    ax2.text(bar.get_x() + bar.get_width()/2., height,
            f'{height:.2f}x', ha='center', va='bottom', fontsize=8)

plt.tight_layout()
plt.savefig('matrix_vector_performance.png', dpi=300, bbox_inches='tight')
plt.show()
print('\nDo thi da duoc luu vao: matrix_vector_performance.png')
