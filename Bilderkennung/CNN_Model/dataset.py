import torch
from torch.utils.data import Dataset
import os
import torchvision
from torchvision.datasets import CocoDetection
from PIL import Image

class CocoDetectionDataset(Dataset):
    def __init__(self, root, annFile, transforms=None, return_masks=False, cache_images=False):
        """
        Args:
            root (str): Directory with images.
            annFile (str): Path to COCO format annotation JSON.
            transforms (callable, optional): Transformations to apply to images and targets.
            return_masks (bool): Whether to return masks (for Mask R-CNN).
        """
        self.root = root
        self.transforms = transforms
        self.return_masks = return_masks
        self.coco = torchvision.datasets.CocoDetection(root=root, annFile=annFile)
        self.ids = list(sorted(self.coco.ids))
        
        #bilder in RAM laden
        self.cache_images = cache_images
        
        if self.cache_images:
            print("Caching images in RAM...")
            self.cached_imgs = {}
            for img_id in self.ids:
                img, _ = self.coco.coco.loadImgs(img_id)[0], None
                img_path = os.path.join(self.root, img['file_name'])
                image = Image.open(img_path).convert("RGB")
                self.cached_imgs[img_id] = image
            print(f"Cached {len(self.cached_imgs)} images.")

    def __getitem__(self, index):
        coco = self.coco
        img_id = self.ids[index]
        ann_ids = coco.coco.getAnnIds(imgIds=img_id)
        anns = coco.coco.loadAnns(ann_ids)
        path = coco.coco.loadImgs(img_id)[0]['file_name']

        # Load image
        img = Image.open(os.path.join(self.root, path))

        boxes = []
        labels = []
        masks = []

        for ann in anns:
            # Bounding box in [x, y, width, height]
            bbox = ann['bbox']
            x_min = bbox[0]
            y_min = bbox[1]
            x_max = x_min + bbox[2]
            y_max = y_min + bbox[3]
            boxes.append([x_min, y_min, x_max, y_max])
            labels.append(ann['category_id'])

            # Optional: load mask if enabled
            if self.return_masks and 'segmentation' in ann:
                rle = coco.coco.annToRLE(ann)
                mask = coco.coco.annToMask(ann)
                masks.append(torch.as_tensor(mask, dtype=torch.uint8))

        target = {
            "boxes": torch.as_tensor(boxes, dtype=torch.float32),
            "labels": torch.as_tensor(labels, dtype=torch.int64),
            "image_id": torch.tensor([img_id]),
            "iscrowd": torch.tensor([ann.get("iscrowd", 0) for ann in anns], dtype=torch.int64),
        }

        # Optional: add masks
        if self.return_masks and masks:
            target["masks"] = torch.stack(masks)

        # Calculate area (required for COCO eval)
        area = []
        for box in boxes:
            area.append((box[2] - box[0]) * (box[3] - box[1]))
        target["area"] = torch.as_tensor(area, dtype=torch.float32)

        if self.transforms is not None:
            img = self.transforms(img)

        return img, target

    def __len__(self):
        return len(self.ids)