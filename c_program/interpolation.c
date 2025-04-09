#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define EPSILON 1e-6

// Function to convert HH:MM to minutes
float convert_to_minutes(int hours, int minutes) {
    return hours * 60 + minutes;
}

// Function to convert minutes to HH:MM
void convert_to_hhmm(float minutes) {
    int total_minutes = (int)minutes;
    int hours = (total_minutes / 60) % 24;
    int mins = total_minutes % 60;

    printf("%02d:%02d", hours, mins);
}

// Lagrange interpolation using selected nearest points
float lagrange_interpolation(float *x, float *y, int n, float xp, int max_points) {
    // Find closest point index
    int closest = 0;
    float min_dist = fabs(xp - x[0]);
    for (int i = 1; i < n; i++) {
        float dist = fabs(xp - x[i]);
        if (dist < min_dist) {
            min_dist = dist;
            closest = i;
        }
    }

    // Determine starting index for subset
    int start = closest - max_points / 2;
    if (start < 0) start = 0;
    if (start + max_points > n) start = n - max_points;
    if (start < 0) start = 0;

    int points = (n < max_points) ? n : max_points;
    if (start + points > n) points = n - start;

    // printf("\nUsing %d nearest points for Lagrange interpolation (starting at index %d)\n", points, start);

    float result = 0.0;

    for (int i = 0; i < points; i++) {
        float term = y[start + i];
        float num = 1.0, den = 1.0;

        for (int j = 0; j < points; j++) {
            if (j != i) {
                num *= (xp - x[start + j]);
                den *= (x[start + i] - x[start + j]);
            }
        }

        if (fabs(den) < EPSILON) {
            printf("Warning: Division by near-zero\n");
            continue;
        }

        term *= (num / den);
        result += term;

        // printf("Point %d (", start + i);
        convert_to_hhmm(x[start + i]);
        // printf(", %.2f): term = %.4f\n", y[start + i], term);
    }

    return result;
}

// Find actual y value closest to given x for error calculation
float find_nearest_actual(float *x, float *y, int n, float xp) {
    float min_diff = fabs(xp - x[0]);
    int idx = 0;
    for (int i = 1; i < n; i++) {
        float diff = fabs(xp - x[i]);
        if (diff < min_diff) {
            min_diff = diff;
            idx = i;
        }
    }
    return y[idx];
}

int main() {
    FILE *file = fopen("../data/pollution_data.csv", "r");
    if (!file) {
        printf("Error: Unable to open file.\n");
        return 1;
    }

    char header[100];
    fgets(header, sizeof(header), file); // Skip header

    // First pass to count rows
    int count = 0;
    float temp1, temp2;
    while (fscanf(file, "%f,%*d,%*f,%f", &temp1, &temp2) == 2) {
        count++;
    }
    rewind(file);
    fgets(header, sizeof(header), file); // Skip header again

    // Dynamic memory allocation
    float *x = (float *)malloc(count * sizeof(float));
    float *y = (float *)malloc(count * sizeof(float));
    if (!x || !y) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    // Load data
    int i = 0;
    while (fscanf(file, "%f,%*d,%*f,%f", &x[i], &y[i]) == 2 && i < count) {
        i++;
    }
    fclose(file);

    printf("Loaded %d data points.\n", count);
    printf("Time\t\tCO2 (ppm)\n");
    for (int j = 0; j < count; j++) {  // Show all data points
        printf("");
        convert_to_hhmm(x[j]);
        printf("\t\t%.2f\n", y[j]);
    }
    
    // Show time range in HH:MM format
    printf("\nData time range: ");
    convert_to_hhmm(x[0]);
    printf(" to ");
    convert_to_hhmm(x[count-1]);
    printf("\n");
    
    // Input time in HH:MM format
    int hours, minutes;
    printf("\nEnter the time in 24-hour format (HH:MM) to predict CO2 level: ");
    if (scanf("%d:%d", &hours, &minutes) != 2) {
        printf("Error: Invalid time format. Please use HH:MM format.\n");
        free(x);
        free(y);
        return 1;
    }
    
    // Validate time input
    if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
        printf("Error: Invalid time values. Hours must be 0-23, minutes must be 0-59.\n");
        free(x);
        free(y);
        return 1;
    }
    
    // Convert input time to minutes for interpolation
    float xp = convert_to_minutes(hours, minutes);
    
    printf("Time entered: %02d:%02d (%.0f minutes)\n", hours, minutes, xp);

    if (xp < x[0] || xp > x[count - 1]) {
        printf("⚠️  Warning: Extrapolation - input time is outside data range (");
        convert_to_hhmm(x[0]);
        printf(" to ");
        convert_to_hhmm(x[count-1]);
        printf(")\n");
    }

    float predicted = lagrange_interpolation(x, y, count, xp, 4);
    float actual = find_nearest_actual(x, y, count, xp);

    float error_percent = fabs((predicted - actual) / actual) * 100;

    printf("\n========== RESULT ==========\n");
    printf("Predicted CO2 at %02d:%02d: %.2f ppm\n", hours, minutes, predicted);
    printf("Nearest actual CO2: %.2f ppm\n", actual);
    printf("Absolute Error: %.2f ppm\n", fabs(predicted - actual));
    printf("Relative Error: %.2f%%\n", error_percent);

    // Free memory
    free(x);
    free(y);

    return 0;
}