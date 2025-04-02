#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX 100  // Maximum data points
#define EPSILON 1e-6  // Small value to check for near-zero

// Linear interpolation - simpler and more stable for this application
float linear_interpolation(float x[], float y[], int n, float xp) {
    // Find the two points that bracket xp
    int i = 0;
    while (i < n - 1 && x[i + 1] < xp) {
        i++;
    }

    if (i == n - 1) {
        // xp is beyond the last point, use the last point value
        return y[n - 1];
    }

    if (i < 0) {
        // xp is before the first point, use the first point value
        return y[0];
    }

    // Perform linear interpolation between x[i] and x[i+1]
    float t = (xp - x[i]) / (x[i + 1] - x[i]);
    return y[i] * (1 - t) + y[i + 1] * t;
}

// Lagrange interpolation (use a local subset of points for better stability)
float lagrange_interpolation(float x[], float y[], int n, float xp, int max_points) {
    // Find closest points to use for interpolation
    int start_idx = 0;
    float min_dist = fabs(xp - x[0]);

    for (int i = 1; i < n; i++) {
        float dist = fabs(xp - x[i]);
        if (dist < min_dist) {
            min_dist = dist;
            start_idx = i;
        }
    }

    // Adjust starting index to get points both before and after xp if possible
    start_idx = start_idx - max_points/2;
    if (start_idx < 0) start_idx = 0;
    if (start_idx + max_points > n) start_idx = n - max_points;
    if (start_idx < 0) start_idx = 0;  // In case n < max_points

    // Number of points to use for interpolation
    int points = (n < max_points) ? n : max_points;
    if (start_idx + points > n) points = n - start_idx;

    printf("\nUsing %d points for Lagrange interpolation (starting at index %d)\n", points, start_idx);

    // Perform Lagrange interpolation with the selected points
    float result = 0.0;

    for (int i = 0; i < points; i++) {
        float term = y[start_idx + i];
        float numerator = 1.0;
        float denominator = 1.0;

        for (int j = 0; j < points; j++) {
            if (j != i) {
                numerator *= (xp - x[start_idx + j]);
                denominator *= (x[start_idx + i] - x[start_idx + j]);
            }
        }

        // Check for division by zero
        if (fabs(denominator) < EPSILON) {
            printf("Warning: Division by near-zero in Lagrange interpolation\n");
            continue;
        }

        term *= (numerator / denominator);
        result += term;

        // Print steps for clarity
        printf("Point %d (%.2f, %.2f): term = %.4f\n",
               start_idx + i, x[start_idx + i], y[start_idx + i], term);
    }

    return result;
}

int main() {
    FILE *file = fopen("../data_processing/pollution_data.csv", "r");
    if (file == NULL) {
        printf("Error: Unable to open file.\n");
        return 1;
    }

    float x[MAX], y[MAX];  // Simplified array structure (don't need difference table)
    int n = 0;

    char header[100];
    fgets(header, sizeof(header), file); // Read and ignore header

    // Read pollution data (Time, CO2)
    while (fscanf(file, "%f,%*d,%*f,%f", &x[n], &y[n]) == 2) {
        n++;
        if (n >= MAX) break;
    }
    fclose(file);

    // Print the data points being used
    printf("Data points being used for interpolation: %d points\n", n);
    printf("Time (s)\tCO2 (ppm)\n");
    for (int i = 0; i < n; i++) {
        printf("%.2f\t\t%.2f\n", x[i], y[i]);
    }

    // Check if data was successfully loaded
    if (n < 2) {
        printf("Error: Not enough data points for interpolation.\n");
        return 1;
    }

    // Get user input for timestamp
    float xp;
    printf("\nEnter the timestamp to predict CO2 level: ");
    scanf("%f", &xp);

    // Check if input is within range
    if (xp < x[0] || xp > x[n-1]) {
        printf("Warning: Interpolation point %.2f is outside data range (%.2f to %.2f).\n",
               xp, x[0], x[n-1]);
        printf("Results may be less reliable due to extrapolation.\n\n");
    }

    // Calculate predicted CO2 with multiple methods for comparison
    float predicted_linear = linear_interpolation(x, y, n, xp);
    float predicted_lagrange = lagrange_interpolation(x, y, n, xp, 4);  // Use at most 4 points

    // Print prediction results
    printf("\n=========== Results ===========\n");
    printf("Timestamp: %.2f seconds\n", xp);
    printf("Linear interpolation: %.2f ppm\n", predicted_linear);
    printf("Lagrange interpolation: %.2f ppm\n", predicted_lagrange);
    printf("Recommended value: %.2f ppm\n", predicted_linear);  // Linear is more stable

    return 0;
}