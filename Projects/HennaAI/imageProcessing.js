// Function to perform Floyd-Steinberg dithering on image data
export function ditherImageData(ctx, width, height) {
  const imageData = ctx.getImageData(0, 0, width, height);
  const pixels = imageData.data;

  // Convert to grayscale first
  for (let i = 0; i < pixels.length; i += 4) {
    const r = pixels[i];
    const g = pixels[i + 1];
    const b = pixels[i + 2];
    // Using luminance weights for better grayscale conversion
    const gray = 0.299 * r + 0.587 * g + 0.114 * b;
    pixels[i] = pixels[i + 1] = pixels[i + 2] = gray;
  }

  // Floyd-Steinberg dithering
  for (let y = 0; y < height; y++) {
    for (let x = 0; x < width; x++) {
      const idx = (y * width + x) * 4;
      const oldPixel = pixels[idx];
      const newPixel = oldPixel < 128 ? 0 : 255;
      pixels[idx] = pixels[idx + 1] = pixels[idx + 2] = newPixel;

      const error = oldPixel - newPixel;

      // Distribute error to neighboring pixels
      if (x + 1 < width) {
        pixels[idx + 4] += (error * 7) / 16;
        pixels[idx + 5] += (error * 7) / 16;
        pixels[idx + 6] += (error * 7) / 16;
      }
      if (y + 1 < height) {
        if (x > 0) {
          pixels[idx + 4 * width - 4] += (error * 3) / 16;
          pixels[idx + 4 * width - 3] += (error * 3) / 16;
          pixels[idx + 4 * width - 2] += (error * 3) / 16;
        }
        pixels[idx + 4 * width] += (error * 5) / 16;
        pixels[idx + 4 * width + 1] += (error * 5) / 16;
        pixels[idx + 4 * width + 2] += (error * 5) / 16;
        if (x + 1 < width) {
          pixels[idx + 4 * width + 4] += (error * 1) / 16;
          pixels[idx + 4 * width + 5] += (error * 1) / 16;
          pixels[idx + 4 * width + 6] += (error * 1) / 16;
        }
      }
    }
  }

  ctx.putImageData(imageData, 0, 0);
}

// Function to build image bytes from canvas context
export function buildImageBytes(ctx, width, height) {
  const rowBytes = Math.ceil(width / 8);
  const totalBytes = rowBytes * height;
  const byteArray = new Uint8Array(totalBytes);

  const imageData = ctx.getImageData(0, 0, width, height).data;
  let byteIndex = 0;

  for (let y = 0; y < height; y++) {
    for (let x = 0; x < rowBytes; x++) {
      let b = 0;
      for (let bit = 0; bit < 8; bit++) {
        const pixelIndex = (y * width + (x * 8 + bit)) * 4;
        if (x * 8 + bit < width) {
          // 0 means black in your threshold
          if (imageData[pixelIndex] === 0) {
            b |= 1 << (7 - bit);
          }
        }
      }
      byteArray[byteIndex++] = b;
    }
  }

  return byteArray;
}
