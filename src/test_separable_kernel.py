import copy

def reflect(M, x):
    if(x < 0): return -x - 1
    if(x >= M): return 2*M - x - 1
    return x


def combined_kernel():
    A = [[1]*5 for  i in range(5)]
    B = copy.deepcopy(A)
    coeffs = [1, 3, 3, 1]
    K = len(coeffs)
    mean_K = K // 2

    for y in range(len(A)):
        for x in range(len(A[y])):
            sum = 0
            for i in range(K):
                shift = i - mean_K
                y1 = reflect(len(A), y + shift)
                sum = sum + coeffs[i]
            A[y][x] = sum

    print(A)
        
    for y in range(len(A)):
        for x in range(len(A[y])):
            sum = 0
            for i in range(K):
                shift = i - mean_K
                x1 = reflect(len(A), y + shift)
                sum = sum + coeffs[i] * A[y][x1]
            B[y][x] = sum

    print(B)


combined_kernel()