//#define _WITH_TEXTURE

using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

using System.IO;
using UnityEditor;
using System.Text;

public class BinarySceneWriter : MonoBehaviour
{
    private List<string> m_strGameObjectNames = new List<string>();
    private List<string> m_strTextureNames = new List<string>();

    void WriteMatrix(BinaryWriter binaryWriter, Matrix4x4 matrix)
    {
        binaryWriter.Write(matrix.m00);
        binaryWriter.Write(matrix.m10);
        binaryWriter.Write(matrix.m20);
        binaryWriter.Write(matrix.m30);
        binaryWriter.Write(matrix.m01);
        binaryWriter.Write(matrix.m11);
        binaryWriter.Write(matrix.m21);
        binaryWriter.Write(matrix.m31);
        binaryWriter.Write(matrix.m02);
        binaryWriter.Write(matrix.m12);
        binaryWriter.Write(matrix.m22);
        binaryWriter.Write(matrix.m32);
        binaryWriter.Write(matrix.m03);
        binaryWriter.Write(matrix.m13);
        binaryWriter.Write(matrix.m23);
        binaryWriter.Write(matrix.m33);
    }

    void WriteLocalMatrix(BinaryWriter binaryWriter, Transform transform)
    {
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(transform.localPosition, transform.localRotation, transform.localScale);
        WriteMatrix(binaryWriter, matrix);
    }

    void WriteWorldMatrix(BinaryWriter binaryWriter, Transform transform)
    {
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(transform.position, transform.rotation, transform.lossyScale);
        WriteMatrix(binaryWriter, matrix);
    }

    void WriteColor(BinaryWriter binaryWriter, Color color)
    {
        binaryWriter.Write(color.r);
        binaryWriter.Write(color.g);
        binaryWriter.Write(color.b);
        binaryWriter.Write(color.a);
    }

    void Write2DVector(BinaryWriter binaryWriter, Vector2 v)
    {
        binaryWriter.Write(v.x);
        binaryWriter.Write(v.y);
    }

    void Write2DVectors(BinaryWriter binaryWriter, string strName, Vector2[] pVectors)
    {
        binaryWriter.Write(strName);
        binaryWriter.Write(pVectors.Length);
        foreach (Vector2 v in pVectors) Write2DVector(binaryWriter, v);
    }

    void Write3DVector(BinaryWriter binaryWriter, Vector3 v)
    {
        binaryWriter.Write(v.x);
        binaryWriter.Write(v.y);
        binaryWriter.Write(v.z);
    }

    void Write3DVectors(BinaryWriter binaryWriter, string strName, Vector3[] pVectors)
    {
        binaryWriter.Write(strName);
        binaryWriter.Write(pVectors.Length);
        foreach (Vector3 v in pVectors) Write3DVector(binaryWriter, v);
    }

    void WriteIntegers(BinaryWriter binaryWriter, int[] pIntegers)
    {
        binaryWriter.Write(pIntegers.Length);
        foreach (int i in pIntegers) binaryWriter.Write(i);
    }

    void WriteIntegers(BinaryWriter binaryWriter, string strName, int[] pIntegers)
    {
        binaryWriter.Write(strName);
        binaryWriter.Write(pIntegers.Length);
        foreach (int i in pIntegers) binaryWriter.Write(i);
    }

    void WriteBoundingBox(BinaryWriter binaryWriter, string strName, Bounds bounds)
    {
        binaryWriter.Write(strName);
        Write3DVector(binaryWriter, bounds.center);
        Write3DVector(binaryWriter, bounds.extents);
    }

    void WriteSubMeshes(BinaryWriter binaryWriter, string strName, Mesh mesh)
    {
        binaryWriter.Write(strName);
        int nSubMeshes = mesh.subMeshCount;
        binaryWriter.Write(nSubMeshes);
        for (int i = 0; i < nSubMeshes; i++)
        {
            uint nSubMeshStart = mesh.GetIndexStart(i);
            uint nSubMeshIndices = mesh.GetIndexCount(i);
            int[] pSubMeshIndices = mesh.GetIndices(i);
            binaryWriter.Write(nSubMeshStart);
            binaryWriter.Write(nSubMeshIndices);
            WriteIntegers(binaryWriter, pSubMeshIndices);
        }
    }

    void WriteMaterials(BinaryWriter binaryWriter, string strName, Material[] materials)
    {
        binaryWriter.Write(strName);
        binaryWriter.Write(materials.Length);
        for (int i = 0; i < materials.Length; i++)
        {
            Color mainColor = materials[i].GetColor("_Color");
            WriteColor(binaryWriter, mainColor);
#if (_WITH_TEXTURE)
            Texture albedoTexture = materials[i].GetTexture("_MainTex");
            if (albedoTexture)
            {
                binaryWriter.Write("<AlbedoTextureName>:");
                binaryWriter.Write(string.Copy(albedoTexture.name).Replace(" ", "_"));
            }
            else
            {
                binaryWriter.Write("<Null>:");
            }
#endif
            Color emissionColor = materials[i].GetColor("_EmissionColor");
            WriteColor(binaryWriter, emissionColor);
#if (_WITH_TEXTURE)
            Texture emissionTexture = materials[i].GetTexture("_EmissionMap");
            if (emissionTexture)
            {
                binaryWriter.Write("<EmissionTextureName>:");
                binaryWriter.Write(string.Copy(emissionTexture.name).Replace(" ", "_"));
            }
            else
            {
                binaryWriter.Write("<Null>:");
            }
#endif
        }
    }

    void WriteMesh(Mesh mesh, string strObjectName)
    {
        BinaryWriter binaryWriter = new BinaryWriter(File.Open(strObjectName + ".bin", FileMode.Create));

        WriteBoundingBox(binaryWriter, "<BoundingBox>:", mesh.bounds); //AABB

        Write3DVectors(binaryWriter, "<Vertices>:", mesh.vertices);
        Write3DVectors(binaryWriter, "<Normals>:", mesh.normals);
#if (_WITH_TEXTURE)
        Write2DVectors(binaryWriter, "<TextureCoords>:", mesh.uv);
#endif
        WriteIntegers(binaryWriter, "<Indices>:", mesh.triangles);

        WriteSubMeshes(binaryWriter, "<SubMeshes>:", mesh);

        binaryWriter.Flush();
        binaryWriter.Close();
    }
    
    bool FindTextureByName(string strTextureName)
    {
        for (int i = 0; i < m_strTextureNames.Count; i++)
        {
            if (m_strTextureNames.Contains(strTextureName)) return (true);
        }
        m_strTextureNames.Add(strTextureName);
        return (false);
    }

    bool FindObjectByName(string strObjectName)
    {
        for (int i = 0; i < m_strGameObjectNames.Count; i++)
        {
            if (m_strGameObjectNames.Contains(strObjectName)) return (true);
        }
        m_strGameObjectNames.Add(strObjectName);
        return (false);
    }

    void WriteFrameInfo(BinaryWriter sceneBinaryWriter, Transform transform)
    {
        MeshFilter meshFilter = transform.gameObject.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = transform.gameObject.GetComponent<MeshRenderer>();

        if (meshFilter && meshRenderer)
        {
            string strObjectName = string.Copy(transform.name).Replace(" ", "_");
            sceneBinaryWriter.Write("<GameObject>:");
            sceneBinaryWriter.Write(strObjectName);
            WriteMaterials(sceneBinaryWriter, "<Materials>:", meshRenderer.materials);
            WriteWorldMatrix(sceneBinaryWriter, transform);

            if (FindObjectByName(strObjectName) == false) WriteMesh(meshFilter.mesh, strObjectName);
        }
    }

    void WriteFrameHierarchy(BinaryWriter sceneBinaryWriter, Transform transform)
    {
        WriteFrameInfo(sceneBinaryWriter, transform);

        for (int k = 0; k < transform.childCount; k++)
        {
            WriteFrameHierarchy(sceneBinaryWriter, transform.GetChild(k));
        }
    }

    int GetChildObjectsCount(Transform transform)
    {
        int nChilds = 0;
        MeshFilter meshFilter = transform.gameObject.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = transform.gameObject.GetComponent<MeshRenderer>();
        if (meshFilter && meshRenderer) nChilds++;

        for (int k = 0; k < transform.childCount; k++) nChilds += GetChildObjectsCount(transform.GetChild(k));

        return (nChilds);
    }

    int GetAllGameObjectsCount()
    {
        int nGameObjects = 0;

        Scene scene = transform.gameObject.scene;
        GameObject[] rootGameObjects = scene.GetRootGameObjects();
        foreach (GameObject gameObject in rootGameObjects) nGameObjects += GetChildObjectsCount(gameObject.transform);

        return (nGameObjects);
    }

    int GetTexturesCount(Material[] materials)
    {
        int nTextures = 0;
        for (int i = 0; i < materials.Length; i++)
        {
            Texture albedoTexture = materials[i].GetTexture("_MainTex"); //materials[i].mainTexture
            if (albedoTexture) nTextures++;
            Texture emissionTexture = materials[i].GetTexture("_EmissionMap"); 
            if (albedoTexture) nTextures++;
        }
        return (nTextures);
    }

    int GetTexturesCount(Transform transform)
    {
        int nTextures = 0;
        string strObjectName = string.Copy(transform.name).Replace(" ", "_");
        if (FindTextureByName(strObjectName) == false)
        {
            MeshRenderer meshRenderer = transform.gameObject.GetComponent<MeshRenderer>();
            if (meshRenderer) nTextures = GetTexturesCount(meshRenderer.materials);
        }
        for (int k = 0; k < transform.childCount; k++) nTextures += GetTexturesCount(transform.GetChild(k));
        return (nTextures);
    }

    int GetAllTexturesCount()
    {
        int nTextures = 0;
        Scene scene = transform.gameObject.scene;
        GameObject[] rootGameObjects = scene.GetRootGameObjects();
        foreach (GameObject gameObject in rootGameObjects) nTextures += GetTexturesCount(gameObject.transform);
        return (nTextures);
    }

    void Start()
    {
        BinaryWriter sceneBinaryWriter = new BinaryWriter(File.Open("Scene.bin", FileMode.Create));

        int nGameObjects = GetAllGameObjectsCount();
        sceneBinaryWriter.Write("<GameObjects>:");
        sceneBinaryWriter.Write(nGameObjects);

#if (_WITH_TEXTURE)
        int nTextures = GetAllTexturesCount();
        sceneBinaryWriter.Write("<Textures>:");
        sceneBinaryWriter.Write(nTextures);
#endif
        Scene scene = transform.gameObject.scene;
        GameObject[] rootGameObjects = scene.GetRootGameObjects();
        foreach (GameObject gameObject in rootGameObjects) WriteFrameHierarchy(sceneBinaryWriter, gameObject.transform);

        sceneBinaryWriter.Flush();
        sceneBinaryWriter.Close();

        print("Mesh Write Completed");
    }
}
